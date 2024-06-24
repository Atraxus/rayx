#pragma once

#include "Common.h"
#include "Complex.h"
#include "Constants.h"

namespace RAYX {

using Stokes = dvec4;
using Field = cvec3;
using LocalField = cvec2;

struct Coeffs {
    double s;
    double p;
};

struct ComplexCoeffs {
    complex::Complex s;
    complex::Complex p;
};

RAYX_FUNC
inline double get_angle(glm::dvec3 a, glm::dvec3 b) {
    return glm::acos(glm::dot(a, b));
}

RAYX_FUNC
inline complex::Complex get_refract_angle(const complex::Complex incident_angle, const complex::Complex ior_i, const complex::Complex ior_t) {
    return complex::asin((ior_i / ior_t) * complex::sin(incident_angle));
}

RAYX_FUNC
inline complex::Complex get_brewsters_angle(const complex::Complex ior_i, const complex::Complex ior_t) {
    return complex::atan(ior_t / ior_i);
}

RAYX_FUNC
inline complex::Complex get_critical_angle(const complex::Complex ior_i, const complex::Complex ior_t) {
    return complex::asin(ior_t / ior_i);
}

RAYX_FUNC
inline ComplexCoeffs get_reflect_amplitude(const complex::Complex incident_angle, const complex::Complex refract_angle, const complex::Complex ior_i, const complex::Complex ior_t) {
    const auto cos_i = complex::cos(incident_angle);
    const auto cos_t = complex::cos(refract_angle);

    const auto s = (ior_i * cos_i - ior_t * cos_t) / (ior_i * cos_i + ior_t * cos_t);
    const auto p = (ior_t * cos_i - ior_i * cos_t) / (ior_t * cos_i + ior_i * cos_t);

    return {
        .s = s,
        .p = p,
    };
}

RAYX_FUNC
inline ComplexCoeffs get_refract_amplitude(const complex::Complex incident_angle, const complex::Complex refract_angle, const complex::Complex ior_i, const complex::Complex ior_t) {
    const auto cos_i = complex::cos(incident_angle);
    const auto cos_t = complex::cos(refract_angle);

    const auto s = (2.0 * ior_i * cos_i) / (ior_i * cos_i + ior_t * cos_t);
    const auto p = (2.0 * ior_i * cos_i) / (ior_t * cos_i + ior_i * cos_t);

    return {
        .s = s,
        .p = p,
    };
}

RAYX_FUNC
inline Coeffs get_reflect_intensity(const ComplexCoeffs reflect_amplitude) {
    const auto s = (reflect_amplitude.s * complex::conj(reflect_amplitude.s)).real();
    const auto p = (reflect_amplitude.p * complex::conj(reflect_amplitude.p)).real();

    return {
        .s = s,
        .p = p,
    };
}

RAYX_FUNC
inline Coeffs get_refract_intensity(ComplexCoeffs refract_amplitude, const complex::Complex incident_angle, const complex::Complex refract_angle, const complex::Complex ior_i, const complex::Complex ior_t) {
    const auto r = ((ior_t * complex::cos(refract_angle)) / (ior_i * complex::cos(incident_angle))).real();

    const auto s = r * (refract_amplitude.s * complex::conj(refract_amplitude.s)).real();
    const auto p = r * (refract_amplitude.p * complex::conj(refract_amplitude.p)).real();

    return {
        .s = s,
        .p = p,
    };
}

RAYX_FUNC
inline cmat3 get_jones_matrix(const ComplexCoeffs amplitude) {
    return {
        amplitude.s, 0, 0,
        0, amplitude.p, 0,
        0, 0, 1,
    };
}

RAYX_FUNC
inline cmat3 get_polarization_matrix(
    const glm::dvec3 incident_vec,
    const glm::dvec3 reflect_or_refract_vec,
    const glm::dvec3 normal_vec,
    const ComplexCoeffs amplitude
) {
    const auto s0 = glm::normalize(glm::cross(incident_vec, -normal_vec));
    const auto s1 = s0;
    const auto p0 = glm::cross(incident_vec, s0);
    const auto p1 = glm::cross(reflect_or_refract_vec, s0);

    const auto o_out = glm::dmat3(
        s1,
        p1,
        reflect_or_refract_vec
    );

    const auto o_in = glm::dmat3(
        s0.x, p0.x, incident_vec.x,
        s0.y, p0.y, incident_vec.y,
        s0.z, p0.z, incident_vec.z
    );

    const auto jones_matrix = get_jones_matrix(amplitude);

    return o_out * jones_matrix * o_in;
}

RAYX_FUNC
inline cmat3 get_reflect_polarization_matrix_at_normal_incidence(const ComplexCoeffs amplitude) {
    // since no plane of incidence is defined at normal incidence,
    // s and p components are equal and only contain the base reflectivity and a phase shift of 180 degrees
    // here we apply the base reflectivity and phase shift independent of the ray direction to all components
    return {
        amplitude.s, 0, 0,
        0, amplitude.s, 0,
        0, 0, amplitude.s,
    };
}

RAYX_FUNC
inline Field intercept_reflect(
    const Field incident_field,
    const dvec3 incident_vec,
    const dvec3 reflect_vec,
    const dvec3 normal_vec,
    const complex::Complex ior_i,
    const complex::Complex ior_t
) {
    const auto incident_angle = complex::Complex(get_angle(incident_vec, -normal_vec), 0);
    const auto refract_angle = get_refract_angle(incident_angle, ior_i, ior_t);

    const auto reflect_amplitude = get_reflect_amplitude(incident_angle, refract_angle, ior_i, ior_t);

    // TODO: make this more robust
    const auto is_normal_incidence = incident_vec == -normal_vec;
    const auto reflect_polarization_matrix =
        is_normal_incidence
        ? get_reflect_polarization_matrix_at_normal_incidence(reflect_amplitude)
        : get_polarization_matrix(incident_vec, reflect_vec, normal_vec, reflect_amplitude);

    const auto reflect_field = reflect_polarization_matrix * incident_field;
    return reflect_field;
}

RAYX_FUNC
inline double intensity(LocalField field) {
    const auto mag = complex::abs(field);
    return glm::dot(mag, mag);
}

RAYX_FUNC
inline double intensity(Field field) {
    const auto mag = complex::abs(field);
    return glm::dot(mag, mag);
}

RAYX_FUNC
inline double intensity(Stokes stokes) {
    return stokes.x;
}

RAYX_FUNC
inline double degreeOfPolarization(const Stokes stokes) {
    return glm::length(glm::vec3(stokes.y, stokes.z, stokes.w)) / stokes.x;
}

RAYX_FUNC
inline Stokes fieldToStokes(const LocalField field) {
    const auto mag = complex::abs(field);
    const auto theta = complex::arg(field);

    return Stokes(
        mag.x*mag.x + mag.y*mag.y,
        mag.x*mag.x - mag.y*mag.y,
        2.0 * mag.x * mag.y * glm::cos(theta.x - theta.y),
        2.0 * mag.x * mag.y * glm::sin(theta.x - theta.y)
    );
}

RAYX_FUNC
inline Stokes fieldToStokes(const Field field) {
    return fieldToStokes(LocalField(field));
}

RAYX_FUNC
inline LocalField stokesToLocalField(const Stokes stokes) {
    const auto x_real = glm::sqrt((stokes.x + stokes.y) / 2.0);

    const auto y_mag = glm::sqrt((stokes.x - stokes.y) / 2.0);
    const auto y_theta = -1.0 * glm::atan(stokes.w, stokes.z);
    const auto y = complex::polar(y_mag, y_theta);

    return LocalField(
        {x_real, 0},
        y
    );
}

RAYX_FUNC
inline Field stokesToField(const Stokes stokes) {
    return Field(stokesToLocalField(stokes), complex::Complex(0, 0));
}

RAYX_FUNC
inline dmat3 rotationMatrix(dvec3 forward) {
    auto up = dvec3(0, 1, 0);
    dvec3 right;

    if (glm::abs(glm::dot(forward, up)) < .5) {
        right = glm::normalize(glm::cross(forward, up));
        up = glm::normalize(glm::cross(right, forward));
    } else {
        right = dvec3(1, 0, 0);
        up = glm::normalize(glm::cross(forward, right));
        right = glm::normalize(glm::cross(forward, up));
    }

    return dmat3(right, up, forward);
}

RAYX_FUNC
inline dmat3 rotationMatrix(dvec3 forward, dvec3 up) {
    const auto right = glm::cross(forward, up);
    return dmat3(right, up, forward);
}

} // namespace RAYX
