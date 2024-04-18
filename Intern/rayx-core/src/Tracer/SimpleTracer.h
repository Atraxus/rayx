#pragma once

#include <cmath>
#include <cstring>

#include "DeviceTracer.h"
#include "Gather.h"
#include "Scan.h"
#include "Util.h"

#include "RAY-Core.h"
#include "Random.h"
#include "Beamline/Beamline.h"
#include "Material/Material.h"
#include "Shader/DynamicElements.h"

namespace {

struct DynamicElementsKernel {
    template <typename Acc>
    RAYX_FUNC
    void operator() (const Acc& acc, Inv inv) const {
        using Idx = alpaka::Idx<Acc>;
        const Idx gid = alpaka::getIdx<alpaka::Grid, alpaka::Threads>(acc)[0];

        if (gid < static_cast<Idx>(inv.rayData.size()))
            dynamicElements(gid, inv);
    }
};

} // unnamed namespace

namespace RAYX {

/**
 * @brief SimpleTracer sequentially executes tracing in batches the CPU or GPU
 */
template <typename TAcc>
class SimpleTracer : public DeviceTracer {
  private:
    using Acc = TAcc;
    using Cpu = alpaka::DevCpu;

    using Dim = alpaka::Dim<Acc>;
    using Idx = alpaka::Idx<Acc>;
    using Vec = alpaka::Vec<Dim, Idx>;

    using QueueProperty = alpaka::NonBlocking;
    using Queue = alpaka::Queue<Acc, QueueProperty>;

  public:
    SimpleTracer(int deviceIndex);

    BundleHistory trace(
        const Beamline&,
        Sequential sequential,
        uint64_t maxBatchSize,
        int getInputRaysThreadCount,
        unsigned int maxEvents,
        int startEventID
    ) override;

  private:
    struct TraceResult {
        int totalEventsCount;
    };

    template <typename T>
    struct Buffer {
        using Buf = alpaka::Buf<Acc, T, Dim, Idx>;
        using OptBuf = std::optional<Buf>;

        OptBuf buf;
    };

    const int m_deviceIndex;

    struct BeamlineInput {
        Buffer<Element> elements;
        Buffer<int>     materialIndices;
        Buffer<double>  materialData;

        Idx elementsSize;
        Idx materialIndicesSize;
        Idx materialDataSize;
    } m_beamlineInput;

    struct BatchInput {
        Buffer<Ray> rays;

        Idx raysSize;
    } m_batchInput;

    struct BatchOutput {
        Buffer<Idx> compactEventCounts;
        Buffer<Idx> compactEventOffsets;
        Buffer<Ray> compactEvents;
        Buffer<Ray> events;
    } m_batchOutput;

    template <typename T>
    void resizeVectorIfNeeded(std::vector<T>& vec, const Idx size);

    template <typename T>
    void resizeBufferIfNeeded(Queue q, Buffer<T>& buffer, const Idx size);

    template <typename T>
    void transferToBuffer(Queue q, alpaka::DevCpu cpu, Buffer<T>& dst, const T* src, const Idx size);
    template <typename T>
    void transferToBuffer(Queue q, alpaka::DevCpu cpu, Buffer<T>& dst, const std::vector<T>& src, const Idx size);

    template <typename T>
    void transferFromBuffer(Queue q, alpaka::DevCpu cpu, T* dst, Buffer<T>& src, const Idx size);
    template <typename T>
    void transferFromBuffer(Queue q, alpaka::DevCpu cpu, std::vector<T>& dst, Buffer<T>& src, const Idx size);

    TraceResult traceBatch(Queue q, const Idx numInputRays);
};

template <typename Acc>
SimpleTracer<Acc>::SimpleTracer(int deviceIndex) :
    m_deviceIndex(deviceIndex)
{
}

template <typename Acc>
BundleHistory SimpleTracer<Acc>::trace(const Beamline& b, Sequential seq, uint64_t maxBatchSize, int getInputRaysThreadCount, unsigned int maxEvents, int startEventID) {
    RAYX_PROFILE_FUNCTION_STDOUT();
    RAYX_VERB << "maxEvents: " << maxEvents;

    // don't trace if there are no optical elements
    if (b.m_OpticalElements.size() == 0) {
        // an empty history suffices, nothing is happening to the rays!
        BundleHistory result;
        return result;
    }

    // prepare input data
    auto extractElements = [&b] {
        std::vector<Element> elements;
        elements.reserve(b.m_OpticalElements.size());
        for (const auto& e : b.m_OpticalElements)
            elements.push_back(e.m_element);
        return elements;
    };
    const auto elements = extractElements();
    const auto rays = b.getInputRays(getInputRaysThreadCount);
    const auto materialTables = b.calcMinimalMaterialTables();
    const auto randomSeed = randomDouble();

    const auto cpu = getDevice<Cpu>();
    const auto acc = getDevice<Acc>(m_deviceIndex);
    auto q = Queue(acc);

    const auto firstBatchSize = static_cast<Idx>(glm::min(rays.size(), maxBatchSize));
    const auto maxOutputEventsCount = static_cast<Idx>(maxBatchSize * (maxEvents - startEventID));
    const auto initialCompactEventsSize = static_cast<Idx>(maxBatchSize * glm::min(2u, maxEvents));
    auto compactEventCounts = std::vector<Idx>();
    auto compactEventOffsets = std::vector<Idx>();
    auto compactEvents = std::vector<Ray>();
    resizeBufferIfNeeded(q, m_batchOutput.compactEvents, initialCompactEventsSize);
    resizeBufferIfNeeded(q, m_batchOutput.compactEventCounts, firstBatchSize);
    resizeBufferIfNeeded(q, m_batchOutput.compactEventOffsets, firstBatchSize);
    resizeBufferIfNeeded(q, m_batchOutput.events, maxOutputEventsCount);
    m_beamlineInput.elementsSize = static_cast<Idx>(elements.size());
    m_beamlineInput.materialIndicesSize = static_cast<Idx>(materialTables.indexTable.size());
    m_beamlineInput.materialDataSize = static_cast<Idx>(materialTables.materialTable.size());
    transferToBuffer(q, cpu, m_beamlineInput.elements, elements, m_beamlineInput.elementsSize);
    transferToBuffer(q, cpu, m_beamlineInput.materialIndices, materialTables.indexTable, m_beamlineInput.materialIndicesSize);
    transferToBuffer(q, cpu, m_beamlineInput.materialData, materialTables.materialTable, m_beamlineInput.materialDataSize);

    // This will be the complete BundleHistory.
    // All initialized events will have been put into this by the end of this function.
    BundleHistory result;

    // iterate over all batches.
    for (int batch_id = 0; batch_id * maxBatchSize < rays.size(); batch_id++) {
        // `rayIdStart` is the ray-id of the first ray of this batch.
        // All previous batches consisted of `maxBatchSize`-many rays.
        // (Only the last batch might be smaller than maxBatchSize, if the number of rays isn't divisible by maxBatchSize).
        const auto rayIdStart = batch_id * maxBatchSize;
        const auto remaining_rays = rays.size() - batch_id * maxBatchSize;
        // The number of input-rays that we put into this batch.
        // Typically equal to maxBatchSize, except for the last batch.
        const auto batchSize = (maxBatchSize < remaining_rays) ? maxBatchSize : remaining_rays;
        const auto numInputRays = batchSize;

        const auto sequential = (double)(seq == Sequential::Yes);
        m_pushConstants = {
            .rayIdStart = (double)rayIdStart,
            .numRays = (double)rays.size(),
            .randomSeed = randomSeed,
            .maxEvents = (double)maxEvents,
            .sequential = sequential,
            .startEventID = (double)startEventID
        };

        const auto inputRays = rays.data() + rayIdStart;
        m_batchInput.raysSize = static_cast<Idx>(batchSize);
        transferToBuffer(q, cpu, m_batchInput.rays, inputRays, m_batchInput.raysSize);

        // run the actual tracer (GPU/CPU).
        const auto traceResult = traceBatch(q, numInputRays);
        RAYX_LOG << "Traced " << traceResult.totalEventsCount << " events.";

        transferFromBuffer(q, cpu, compactEventCounts, m_batchOutput.compactEventCounts, static_cast<Idx>(numInputRays));
        transferFromBuffer(q, cpu, compactEventOffsets, m_batchOutput.compactEventOffsets, static_cast<Idx>(numInputRays));
        transferFromBuffer(q, cpu, compactEvents, m_batchOutput.compactEvents, static_cast<Idx>(traceResult.totalEventsCount));

        alpaka::wait(q);

        // put all events from the rawBatch to unified `BundleHistory result`.
        {
            RAYX_PROFILE_SCOPE_STDOUT("BundleHistory-calculation");
            for (uint i = 0; i < batchSize; i++) {
                // We now create the Rayhistory for the `i`th ray of the batch:
                auto begin = compactEvents.data() + compactEventOffsets[i];
                auto end = begin + compactEventCounts[i];
                auto hist = RayHistory(begin, end);

                // We put the `hist` for the `i`th ray of the batch into the global `BundleHistory result`.
                result.push_back(hist);
            }
        }
    }

    return result;
}

template <typename Acc>
SimpleTracer<Acc>::TraceResult SimpleTracer<Acc>::traceBatch(Queue q, const Idx numInputRays) {
    RAYX_PROFILE_FUNCTION_STDOUT();

    // reference resources

    auto inv = Inv {
        // shader instance local variables
        .globalInvocationId = {},
        .finalized          = {},
        .ctr                = {},
        .nextEventIndex     = {},

        // buffers
        .rayData            = bufToSpan(*m_batchInput.rays.buf, m_batchInput.raysSize),
        .outputData         = bufToSpan(*m_batchOutput.events.buf),
        .outputRayCounts    = bufToSpan(*m_batchOutput.compactEventCounts.buf),
        .elements           = bufToSpan(*m_beamlineInput.elements.buf, m_beamlineInput.elementsSize),
        .xyznull            = {},
        .matIdx             = bufToSpan(*m_beamlineInput.materialIndices.buf, m_beamlineInput.materialIndicesSize),
        .mat                = bufToSpan(*m_beamlineInput.materialData.buf, m_beamlineInput.materialDataSize),

#ifdef RAYX_DEBUG_MODE
        .d_struct           = {},
#endif

        .pushConstants      = m_pushConstants,
    };

    // execute dynamic elements shader

    alpaka::exec<Acc>(
        q,
        getWorkDivForAcc<Acc>(numInputRays),
        DynamicElementsKernel{},
        inv
    );

    // make output events compact

    auto totalEventsCount = scan_sum<Acc, Idx>(
        q,
        *m_batchOutput.compactEventOffsets.buf,
        *m_batchOutput.compactEventCounts.buf,
        numInputRays
    );

    resizeBufferIfNeeded(q, m_batchOutput.compactEvents, totalEventsCount);

    gather_n<Acc, Ray>(
        q,
        *m_batchOutput.compactEvents.buf,
        *m_batchOutput.events.buf,
        *m_batchOutput.compactEventOffsets.buf,
        *m_batchOutput.compactEventCounts.buf,
        static_cast<Idx>(inv.pushConstants.maxEvents),
        numInputRays
    );

    return TraceResult {
        .totalEventsCount = totalEventsCount,
    };
}

template <typename Acc>
template <typename T>
void SimpleTracer<Acc>::resizeVectorIfNeeded(std::vector<T>& vec, const Idx size) {
    const auto nextPowerOfTwo = glm::pow(2, glm::ceil(glm::log(size) / glm::log(2)));
    vec.resize(nextPowerOfTwo);
}

template <typename Acc>
template <typename T>
void SimpleTracer<Acc>::resizeBufferIfNeeded(Queue q, Buffer<T>& buffer, const Idx size) {
    const auto shouldAlloc = !buffer.buf || alpaka::getExtentProduct(*buffer.buf) < size;
    if (shouldAlloc) {
        const auto nextPowerOfTwo = glm::pow(2, glm::ceil(glm::log(size) / glm::log(2)));
        buffer.buf = alpaka::allocAsyncBufIfSupported<T, Idx>(q, Vec{nextPowerOfTwo});
    }
}

template <typename Acc>
template <typename T>
void SimpleTracer<Acc>::transferToBuffer(Queue q, alpaka::DevCpu cpu, Buffer<T>& dst, const T* src, const Idx size) {
    resizeBufferIfNeeded(q, dst, size);
    auto srcView = alpaka::createView(cpu, src, Vec{size});
    alpaka::memcpy(q, *dst.buf, srcView, Vec{size});
}

template <typename Acc>
template <typename T>
void SimpleTracer<Acc>::transferToBuffer(Queue q, alpaka::DevCpu cpu, Buffer<T>& dst, const std::vector<T>& src, const Idx size) {
    transferToBuffer(q, cpu, dst, src.data(), size);
}

template <typename Acc>
template <typename T>
void SimpleTracer<Acc>::transferFromBuffer(Queue q, alpaka::DevCpu cpu, T* dst, Buffer<T>& src, const Idx size) {
    auto dstView = alpaka::createView(cpu, dst, Vec{size});
    alpaka::memcpy(q, dstView, *src.buf, Vec{size});
}

template <typename Acc>
template <typename T>
void SimpleTracer<Acc>::transferFromBuffer(Queue q, alpaka::DevCpu cpu, std::vector<T>& dst, Buffer<T>& src, const Idx size) {
    resizeVectorIfNeeded(dst, size);
    auto dstView = alpaka::createView(cpu, dst, Vec{size});
    alpaka::memcpy(q, dstView, *src.buf, Vec{size});
}

}  // namespace RAYX
