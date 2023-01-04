from dataclasses import dataclass
# from msilib.schema import Directory
# import re
import xml.etree.ElementTree as ET
import math
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
import numpy as np
import sys


@dataclass
class MetaData:
    numElements = 5
    fileName = "multi_RZP_test"


# Standard RZP Parameters
@dataclass
class RZP:
    name = "Reflection Zoneplate"
    type = "Reflection Zoneplate"
    geometricalShape = 0
    totalWidth = 0.1092372974
    totalWidthB = 0.2567627027
    totalLength = 72.5
    gratingMount = 1
    grazingIncAngle = 2.2
    deviationAngle = 170
    distancePreceding = 90
    azimuthalAngle = 0
    elementOffsetZType = 1
    elementOffsetZ = 0
    meridionalIncidenceBeamDivergence = 0
    meridionalIncidenceFocusDistance = 0
    orderDiffraction = -1
    betaDiffraction = 1
    derivationMethod = 0
    coefficientsFile = ''
    designEnergy = 640
    designOrderDiffraction = -1
    entranceArmLengthSag = 90
    entranceArmLengthMer = 90
    designAlphaAngle = 2.2
    exitArmLengthSag = 400
    exitArmLengthMer = 400
    curvatureType = 0
    longRadius = 0
    shortRadius = 0
    designType = 1
    FresnelZOffset = 0
    designBetaAngle = 1
    imageType = 0
    stretchXdirection = 1
    rzpType = 0
    zDcalc = 0
    xDcalc = 0
    Dz = 301.857
    Dx = 0
    refracMethod = 1
    additionalOrder = 1
    lineProfile = 2
    fullEfficiency = 0
    gratingEfficiency = 1
    blazeAngle = 4
    aspectAngle = 90
    grooveDepth = 10
    grooveRatio = 0.65
    multilayerFourierCoefficients = 11
    multilayerIntegrationSteps = 50
    reflectivityType = 0
    materialSubstrate = 'Ni'
    roughnessSubstrate = 1
    densitySubstrate = 8.876
    surfaceCoating = 0
    numberLayer = 2
    materialCoating1 = ''
    thicknessCoating1 = 0
    densityCoating1 = 0
    materialCoating2 = ''
    thicknessCoating2 = 0
    densityCoating2 = 0
    materialTopLayer = ''
    thicknessTopLayer = 0
    densityTopLayer = 0
    lateralThicknessGradientCoating1 = 0
    gradientC1B1 = 0
    gradientC1B2 = 0
    gradientC1B3 = 0
    gradientC1B4 = 0
    gradientC1B5 = 0
    gradientC1B6 = 0
    gradientC1B7 = 0
    gradientC1B8 = 0
    alignmentError = 1
    translationXerror = 0
    translationYerror = 0
    translationZerror = 0
    rotationXerror = 0
    rotationYerror = 0
    rotationZerror = 0
    slopeError = 1
    profileKind = 2
    profileFile = ''
    slopeErrorSag = 0
    slopeErrorMer = 0
    thermalDistortionAmp = 0
    thermalDistortionSigmaX = 0
    thermalDistortionSigmaZ = 0
    cylindricalBowingAmp = 0
    cylindricalBowingRadius = 0
    worldPosition = np.array([0, 0, 90])
    worldXdirection = np.array([1,   0,          0])
    worldYdirection = np.array([0,   0.999263,  -0.0383878])
    worldZdirection = np.array([0,   0.0383878,  0.999263])

# -------------- XML Generation --------------


def xmlToFile(fileName, xmlString):
    with open(fileName+'.rml', "w") as textFile:
        textFile.write(xmlString)


def indent(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i


def insertParamVec3(root: ET.Element, id: str, enabled: bool, xVal: float, yVal: float, zVal: float):
    param = ET.SubElement(root, 'param', id=id, enabled=enabled)
    x = ET.SubElement(param, 'x')
    x.text = str(xVal)
    y = ET.SubElement(param, 'y')
    y.text = str(yVal)
    z = ET.SubElement(param, 'z')
    z.text = str(zVal)

    return root

# Group Position and Direction


def insertGroupPostion(rzp: RZP, root: ET.Element):
    root = insertParamVec3(root, "worldPosition", "F",
                           rzp.worldPosition[0], rzp.worldPosition[1], rzp.worldPosition[2])
    root = insertParamVec3(root, "worldXdirection", "F",
                           rzp.worldXdirection[0], rzp.worldXdirection[1], rzp.worldXdirection[2])
    root = insertParamVec3(root, "worldYdirection", "F",
                           rzp.worldYdirection[0], rzp.worldYdirection[1], rzp.worldYdirection[2])
    root = insertParamVec3(root, "worldZdirection", "F",
                           rzp.worldZdirection[0], rzp.worldZdirection[1], rzp.worldZdirection[2])

    return root


def insertRZP(root, rzp: RZP):
    xmlRZP = ET.SubElement(root, 'object', name=rzp.name,
                           type="Reflection Zoneplate")

    geoShape = ET.SubElement(
        xmlRZP, 'param', id="geometricalShape", comment="rectangle", enabled="T")
    geoShape.text = str(rzp.geometricalShape)

    totWidA = ET.SubElement(xmlRZP, 'param', id="totalWidth", enabled="T")
    totWidA.text = str(rzp.totalWidth)

    totWidB = ET.SubElement(xmlRZP, 'param', id="totalWidthB", enabled="T")
    totWidB.text = str(rzp.totalWidthB)

    totLen = ET.SubElement(xmlRZP, 'param', id="totalLength", enabled="T")
    totLen.text = str(rzp.totalLength)

    totWin = ET.SubElement(xmlRZP, 'param', id="totalWidth", enabled="T")
    totWin.text = str(rzp.totalWidth)

    gratMount = ET.SubElement(xmlRZP, 'param', id="gratingMount",
                              comment="constant incidence angle", enabled="T")
    gratMount.text = str(rzp.gratingMount)

    grazIncAng = ET.SubElement(
        xmlRZP, 'param', id="grazingIncAngle", enabled="T")
    grazIncAng.text = str(rzp.grazingIncAngle)

    devAng = ET.SubElement(xmlRZP, 'param', id="deviationAngle", enabled="F")
    devAng.text = str(rzp.deviationAngle)

    disPre = ET.SubElement(
        xmlRZP, 'param', id="distancePreceding", enabled="T")
    disPre.text = str(rzp.distancePreceding)

    aziAng = ET.SubElement(xmlRZP, 'param', id="azimuthalAngle", enabled="T")
    aziAng.text = str(rzp.azimuthalAngle)

    elOffZType = ET.SubElement(xmlRZP, 'param', id="elementOffsetZType",
                               comment="by beam divergence and distance ", enabled="T")
    elOffZType.text = str(rzp.elementOffsetZType)

    elOffZ = ET.SubElement(xmlRZP, 'param', id="elementOffsetZ", enabled="F")
    elOffZ.text = str(rzp.elementOffsetZ)

    meriIncBeamDiv = ET.SubElement(
        xmlRZP, 'param', id="meridionalIncidenceBeamDivergence", enabled="T")
    meriIncBeamDiv.text = str(rzp.meridionalIncidenceBeamDivergence)

    meriIncFocDis = ET.SubElement(
        xmlRZP, 'param', id="meridionalIncidenceFocusDistance", enabled="T")
    meriIncFocDis.text = str(rzp.meridionalIncidenceFocusDistance)

    ordDiff = ET.SubElement(
        xmlRZP, 'param', id="orderDiffraction", enabled="T")
    ordDiff.text = str(rzp.orderDiffraction)

    betaDiff = ET.SubElement(
        xmlRZP, 'param', id="betaDiffraction", enabled="T")
    betaDiff.text = str(rzp.betaDiffraction)

    derMeth = ET.SubElement(
        xmlRZP, 'param', id="derivationMethod", comment="Formulas", enabled="T")
    derMeth.text = str(rzp.derivationMethod)

    coeFile = ET.SubElement(
        xmlRZP, 'param', id="coefficientsFile", relative="", enabled="F")
    coeFile.text = str(rzp.coefficientsFile)

    desEnergy = ET.SubElement(xmlRZP, 'param', id="designEnergy", enabled="T")
    desEnergy.text = str(rzp.designEnergy)

    desOrdDiff = ET.SubElement(
        xmlRZP, 'param', id="designOrderDiffraction", auto="T", enabled="T")
    desOrdDiff.text = str(rzp.designOrderDiffraction)

    entArmLenSag = ET.SubElement(
        xmlRZP, 'param', id="entranceArmLengthSag", enabled="T")
    entArmLenSag.text = str(rzp.entranceArmLengthSag)

    entArmLenMer = ET.SubElement(
        xmlRZP, 'param', id="entranceArmLengthMer", enabled="T")
    entArmLenMer.text = str(rzp.entranceArmLengthMer)

    desAlphaAng = ET.SubElement(
        xmlRZP, 'param', id="designAlphaAngle", enabled="T")
    desAlphaAng.text = str(rzp.designAlphaAngle)

    exitArmLenSag = ET.SubElement(
        xmlRZP, 'param', id="exitArmLengthSag", enabled="T")
    exitArmLenSag.text = str(rzp.exitArmLengthSag)

    exitArmLenMer = ET.SubElement(
        xmlRZP, 'param', id="exitArmLengthMer", enabled="T")
    exitArmLenMer.text = str(rzp.exitArmLengthMer)

    curveType = ET.SubElement(
        xmlRZP, 'param', id="curvatureType", comment="planar", enabled="T")
    curveType.text = str(rzp.curvatureType)

    longRad = ET.SubElement(xmlRZP, 'param', id="longRadius", enabled="F")
    longRad.text = str(rzp.longRadius)

    shortRad = ET.SubElement(xmlRZP, 'param', id="shortRadius", enabled="F")
    shortRad.text = str(rzp.shortRadius)

    desType = ET.SubElement(xmlRZP, 'param', id="designType",
                            comment="use Design Angle beta", enabled="T")
    desType.text = str(rzp.designType)

    fresZOff = ET.SubElement(xmlRZP, 'param', id="FresnelZOffset", enabled="F")
    fresZOff.text = str(rzp.FresnelZOffset)

    desBetaAng = ET.SubElement(
        xmlRZP, 'param', id="designBetaAngle", enabled="T")
    desBetaAng.text = str(rzp.designBetaAngle)

    imType = ET.SubElement(xmlRZP, 'param', id="imageType",
                           comment="point to point", enabled="T")
    imType.text = str(rzp.imageType)

    strXdir = ET.SubElement(
        xmlRZP, 'param', id="stretchXdirection", enabled="F")
    strXdir.text = str(rzp.stretchXdirection)

    rzpTyp = ET.SubElement(xmlRZP, 'param', id="rzpType",
                           comment="elliptical (standard)", enabled="T")
    rzpTyp.text = str(rzp.rzpType)

    zDCal = ET.SubElement(xmlRZP, 'param', id="zDcalc", enabled="T")
    zDCal.text = str(rzp.zDcalc)

    xDCal = ET.SubElement(xmlRZP, 'param', id="xDcalc", enabled="T")
    xDCal.text = str(rzp.xDcalc)

    dZ = ET.SubElement(xmlRZP, 'param', id="Dz", enabled="T")
    dZ.text = str(rzp.Dz)

    dX = ET.SubElement(xmlRZP, 'param', id="Dx", enabled="T")
    dX.text = str(rzp.Dx)

    refracMet = ET.SubElement(
        xmlRZP, 'param', id="refracMethod", comment="2D (new)", enabled="T")
    refracMet.text = str(rzp.refracMethod)

    addOrd = ET.SubElement(
        xmlRZP, 'param', id="additionalOrder", comment="on", enabled="T")
    addOrd.text = str(rzp.additionalOrder)

    lineProf = ET.SubElement(
        xmlRZP, 'param', id="lineProfile", comment="laminar", enabled="T")
    lineProf.text = str(rzp.lineProfile)

    fullEff = ET.SubElement(
        xmlRZP, 'param', id="fullEfficiency", comment="off", enabled="T")
    fullEff.text = str(rzp.fullEfficiency)

    gratEff = ET.SubElement(
        xmlRZP, 'param', id="gratingEfficiency", enabled="F")
    gratEff.text = str(rzp.gratingEfficiency)

    blazAng = ET.SubElement(xmlRZP, 'param', id="blazeAngle", enabled="F")
    blazAng.text = str(rzp.blazeAngle)

    apectAng = ET.SubElement(xmlRZP, 'param', id="aspectAngle", enabled="T")
    apectAng.text = str(rzp.aspectAngle)

    grooveDep = ET.SubElement(xmlRZP, 'param', id="grooveDepth", enabled="T")
    grooveDep.text = str(rzp.grooveDepth)

    grooveRat = ET.SubElement(xmlRZP, 'param', id="grooveRatio", enabled="T")
    grooveRat.text = str(rzp.grooveRatio)

    multiLayFC = ET.SubElement(
        xmlRZP, 'param', id="multilayerFourierCoefficients", auto="T", enabled="T")
    multiLayFC.text = str(rzp.multilayerFourierCoefficients)

    multiLayIntSteps = ET.SubElement(
        xmlRZP, 'param', id="multilayerIntegrationSteps", auto="T", enabled="T")
    multiLayIntSteps.text = str(rzp.multilayerIntegrationSteps)

    reflecType = ET.SubElement(
        xmlRZP, 'param', id="reflectivityType", comment="100%", enabled="T")
    reflecType.text = str(rzp.reflectivityType)

    matSub = ET.SubElement(
        xmlRZP, 'param', id="materialSubstrate", enabled="F")
    matSub.text = str(rzp.materialSubstrate)

    roughSub = ET.SubElement(
        xmlRZP, 'param', id="roughnessSubstrate", enabled="F")
    roughSub.text = str(rzp.roughnessSubstrate)

    denSub = ET.SubElement(
        xmlRZP, 'param', id="densitySubstrate", auto="T", enabled="F")
    denSub.text = str(rzp.densitySubstrate)

    surfCoat = ET.SubElement(
        xmlRZP, 'param', id="surfaceCoating", comment="Substrate only", enabled="F")
    surfCoat.text = str(rzp.surfaceCoating)

    numLayer = ET.SubElement(xmlRZP, 'param', id="numberLayer", enabled="F")
    numLayer.text = str(rzp.numberLayer)

    matCoat1 = ET.SubElement(
        xmlRZP, 'param', id="materialCoating1", enabled="F")
    matCoat1.text = str(rzp.materialCoating1)

    thickCoat1 = ET.SubElement(
        xmlRZP, 'param', id="thicknessCoating1", enabled="F")
    thickCoat1.text = str(rzp.thicknessCoating1)

    denseCoat1 = ET.SubElement(
        xmlRZP, 'param', id="densityCoating1", auto="T", enabled="F")
    denseCoat1.text = str(rzp.densityCoating1)

    matCoat2 = ET.SubElement(
        xmlRZP, 'param', id="materialCoating2", enabled="F")
    matCoat2.text = str(rzp.materialCoating2)

    thickCoat2 = ET.SubElement(
        xmlRZP, 'param', id="thicknessCoating2", enabled="F")
    thickCoat2.text = str(rzp.thicknessCoating2)

    denseCoat2 = ET.SubElement(
        xmlRZP, 'param', id="densityCoating2", auto="T", enabled="F")
    denseCoat2.text = str(rzp.densityCoating2)

    matTopLay = ET.SubElement(
        xmlRZP, 'param', id="materialTopLayer", enabled="F")
    matTopLay.text = str(rzp.materialTopLayer)

    thickTopLay = ET.SubElement(
        xmlRZP, 'param', id="thicknessTopLayer", enabled="F")
    thickTopLay.text = str(rzp.thicknessTopLayer)

    denTopLay = ET.SubElement(
        xmlRZP, 'param', id="densityTopLayer", auto="T", enabled="F")
    denTopLay.text = str(rzp.densityTopLayer)

    latThickGradCoat1 = ET.SubElement(
        xmlRZP, 'param', id="lateralThicknessGradientCoating1", comment="No", enabled="F")
    latThickGradCoat1.text = str(rzp.lateralThicknessGradientCoating1)

    gradC1B1 = ET.SubElement(xmlRZP, 'param', id="gradientC1B1", enabled="F")
    gradC1B1.text = str(rzp.gradientC1B1)

    gradC1B2 = ET.SubElement(xmlRZP, 'param', id="gradientC1B2", enabled="F")
    gradC1B2.text = str(rzp.gradientC1B2)

    gradC1B3 = ET.SubElement(xmlRZP, 'param', id="gradientC1B3", enabled="F")
    gradC1B3.text = str(rzp.gradientC1B3)

    gradC1B4 = ET.SubElement(xmlRZP, 'param', id="gradientC1B4", enabled="F")
    gradC1B4.text = str(rzp.gradientC1B4)

    gradC1B5 = ET.SubElement(xmlRZP, 'param', id="gradientC1B5", enabled="F")
    gradC1B5.text = str(rzp.gradientC1B5)

    gradC1B6 = ET.SubElement(xmlRZP, 'param', id="gradientC1B6", enabled="F")
    gradC1B6.text = str(rzp.gradientC1B6)

    gradC1B7 = ET.SubElement(xmlRZP, 'param', id="gradientC1B7", enabled="F")
    gradC1B7.text = str(rzp.gradientC1B7)

    gradC1B8 = ET.SubElement(xmlRZP, 'param', id="gradientC1B8", enabled="F")
    gradC1B8.text = str(rzp.gradientC1B8)

    aliError = ET.SubElement(
        xmlRZP, 'param', id="alignmentError", comment="No", enabled="T")
    aliError.text = str(rzp.alignmentError)

    tranXError = ET.SubElement(
        xmlRZP, 'param', id="translationXerror", enabled="F")
    tranXError.text = str(rzp.translationXerror)

    tranYError = ET.SubElement(
        xmlRZP, 'param', id="translationYerror", enabled="F")
    tranYError.text = str(rzp.translationYerror)

    tranZError = ET.SubElement(
        xmlRZP, 'param', id="translationZerror", enabled="F")
    tranZError.text = str(rzp.translationZerror)

    rotXError = ET.SubElement(
        xmlRZP, 'param', id="rotationXerror", enabled="F")
    rotXError.text = str(rzp.rotationXerror)

    rotYError = ET.SubElement(
        xmlRZP, 'param', id="rotationYerror", enabled="F")
    rotYError.text = str(rzp.rotationYerror)

    rotZError = ET.SubElement(
        xmlRZP, 'param', id="rotationZerror", enabled="F")
    rotZError.text = str(rzp.rotationZerror)

    sloError = ET.SubElement(
        xmlRZP, 'param', id="slopeError", comment="No", enabled="T")
    sloError.text = str(rzp.slopeError)

    proKind = ET.SubElement(xmlRZP, 'param', id="profileKind",
                            comment="no Profile", enabled="F")
    proKind.text = str(rzp.profileKind)

    proFile = ET.SubElement(
        xmlRZP, 'param', id="profileFile", relative="", enabled="F")
    proFile.text = str(rzp.profileFile)

    sloErrorSag = ET.SubElement(
        xmlRZP, 'param', id="slopeErrorSag", enabled="F")
    sloErrorSag.text = str(rzp.slopeErrorSag)

    sloErrorMer = ET.SubElement(
        xmlRZP, 'param', id="slopeErrorMer", enabled="F")
    sloErrorMer.text = str(rzp.slopeErrorMer)

    therDistAmp = ET.SubElement(
        xmlRZP, 'param', id="thermalDistortionAmp", enabled="F")
    therDistAmp.text = str(rzp.thermalDistortionAmp)

    therDistSigX = ET.SubElement(
        xmlRZP, 'param', id="thermalDistortionSigmaX", enabled="F")
    therDistSigX.text = str(rzp.thermalDistortionSigmaX)

    therDistSigZ = ET.SubElement(
        xmlRZP, 'param', id="thermalDistortionSigmaZ", enabled="F")
    therDistSigZ.text = str(rzp.thermalDistortionSigmaZ)

    cylBowAmp = ET.SubElement(
        xmlRZP, 'param', id="cylindricalBowingAmp", enabled="F")
    cylBowAmp.text = str(rzp.cylindricalBowingAmp)

    cylBowRad = ET.SubElement(
        xmlRZP, 'param', id="cylindricalBowingRadius", enabled="F")
    cylBowRad.text = str(rzp.cylindricalBowingRadius)

    # xmlRZP Position and Direction
    xmlRZP = insertGroupPostion(rzp, xmlRZP)

    return root


# -------------- RZP Calculations --------------

def rotateYDeg(prevDir: np.array, alpha: float, iterDirection: int):
    # Rotation matrix around y axis (clockwise)
    # alpha = angle in degrees
    # iterDirection = 1 for clockwise, -1 for counter-clockwise

    # Convert alpha to radians
    alpha = math.radians(alpha)

    # Rotation matrix
    rotMat = np.array([[math.cos(alpha), 0, math.sin(alpha)],
                       [0, 1, 0],
                       [-math.sin(alpha), 0, math.cos(alpha)]])

    if (iterDirection == 1):
        # Clockwise rotation
        newDir = np.dot(rotMat, prevDir)
    elif (iterDirection == -1):
        # Counter-clockwise rotation
        newDir = np.dot(rotMat.transpose(), prevDir)

    return newDir


def reflectPointOverLine(point: np.array, linePoint: np.array, lineDir: np.array):
    # Reflects a point over a line
    # point = point to be reflected
    # linePoint = point on the line
    # lineDir = direction of the line

    # Calculate the distance from the point to the line
    dist = np.dot(lineDir, point - linePoint)

    # Calculate the reflected point
    newPoint = point - 2 * dist * lineDir

    return newPoint


def getOrthogonalVector(vector: np.array):
    # Returns an orthogonal vector to the input vector in 3D space
    # vector = input vector

    k = np.random.randn(3)
    k -= k.dot(vector) * vector / np.linalg.norm(vector)**2

    return k


def getParallelLine(dist: float, linePoint: np.array, lineDir: np.array):
    # Calculates a parallel line to a given line in 3D space
    # dist = distance between the lines
    # linePoint = point on the line
    # lineDir = direction of the line

    # Calculate orthogonal vector to the line (solve dot product = 0)
    orthoDir = getOrthogonalVector(lineDir)

    # Calculate the new point on the line
    newPoint = linePoint + dist * orthoDir

    return newPoint


def getAngleBetweenVectors(vector1: np.array, vector2: np.array):
    # Calculates the angle in degrees between two vectors in 3D space
    # vector1 = first vector
    # vector2 = second vector

    # Calculate the angle between the vectors
    angle = math.acos(np.dot(vector1, vector2) /
                      (np.linalg.norm(vector1) * np.linalg.norm(vector2)))

    # Convert the angle to degrees
    angle = math.degrees(angle)

    return angle


def reflectRZP(baseRZP: RZP, iterDirection: int, dist: float):
    newRZP = RZP()
    rzpPos = np.array([baseRZP.worldPosition[0],
                       baseRZP.worldPosition[1],
                       baseRZP.worldPosition[2]])
    rzpDir = np.array([baseRZP.worldXdirection,
                       baseRZP.worldYdirection,
                       baseRZP.worldZdirection])

    if (iterDirection == 1):
        B = np.array([rzpPos[0] - baseRZP.totalWidth / 2,
                      rzpPos[1],
                      rzpPos[2] + baseRZP.totalLength / 2])
        C = np.array([rzpPos[0] + baseRZP.totalWidth / 2,
                      rzpPos[1],
                      rzpPos[2] + baseRZP.totalLength / 2])
        dir = B - C
        pointBC = (B + C) / 2
        reflectionLine = getParallelLine(dist, pointBC, dir)
    else:
        A = np.array([rzpPos[0] - baseRZP.totalWidthB / 2,
                      rzpPos[1],
                      rzpPos[2] + baseRZP.totalLength / 2])
        D = np.array([rzpPos[0] + baseRZP.totalWidthB / 2,
                      rzpPos[1],
                      rzpPos[2] + baseRZP.totalLength / 2])
        dir = A - D
        pointAD = (A + D) / 2
        reflectionLine = getParallelLine(dist, pointAD, dir)

    # Normalize reflection vector
    reflectionLine = reflectionLine / np.linalg.norm(reflectionLine)

    newRZP.worldPosition = reflectPointOverLine(
        rzpPos, reflectionLine, dir)

    rotation = getAngleBetweenVectors(rzpDir[2], reflectionLine) * 2
    newRZPDir = rotateYDeg(rzpDir, rotation, iterDirection)
    newRZP.worldXDirection = newRZPDir[0]
    newRZP.worldYDirection = newRZPDir[1]
    newRZP.worldZDirection = newRZPDir[2]

    # Check if still basis
    check1 = np.dot(newRZP.worldZdirection, newRZP.worldYdirection)
    check2 = np.dot(newRZP.worldZdirection, newRZP.worldXdirection)
    check3 = np.dot(newRZP.worldYdirection, newRZP.worldXdirection)
    if (check1 >= 0.0001 or check2 >= 0.0001 or check3 >= 0.0001):
        print("ERROR: RZP is not basis anymore!")

    return newRZP


def calcRZPs(numRZPs: int, baseRZP: RZP, iterDirection: int, extraSpace: float = 0.0):
    # Calculates the positions and directions of the RZPs
    # numRZPs = number of RZPs
    # baseRZP = base RZP
    # iterDirection = 1 for clockwise, -1 for counter-clockwise
    # extraSpace = extra space between the RZPs

    rzps = []

    if (numRZPs % 2 == 0):
        # Even number of RZPs
        # Calculate temporary RZP to orient the rest
        # TODO(Jannis)
        print("Even number of RZPs not implemented yet")
    else:
        # Odd number of RZPs
        rzps.append(baseRZP)
        for i in range(1, numRZPs):
            rzps.append(reflectRZP(
                rzps[i - 1], iterDirection, extraSpace))
        if (iterDirection == -1):
            rzps.pop(0)
            rzps.reverse()

    return rzps


def main():
    # RZP Data
    meta = MetaData()
    baseRZP = RZP()

    # Optionally define extra space between RZPs (choose one)
    # -s    millimeters, for distance between elements
    # -r    radians, for increased angle    ! not implemented yet !
    # -d    degrees, for increased angle    ! not implemented yet !
    isAngle = False
    extraSpace = 0.0
    if (len(sys.argv) == 3):
        if (sys.argv[1] == "-s"):
            extraSpace = float(sys.argv[2])
        elif (sys.argv[1] == "-r"):
            isAngle = True
            extraSpace = float(sys.argv[2])
        elif (sys.argv[1] == "-d"):
            isAngle = True
            extraSpace = math.radians(float(sys.argv[2]))
        else:
            print("Error: Unknown argument")
            return

    # create RZP Group
    root = ET.Element('group')
    root = insertGroupPostion(baseRZP, root)

    # calculate positions and directions
    if not (isAngle):
        leftPositions, leftDirections = calcRZPs(
            meta.numElements, baseRZP, -1, extraSpace)
        rightPositions, rightDirections = calcRZPs(
            meta.numElements, baseRZP, 1, extraSpace)

    # concatenate left and right positions and directions
    leftPositions.reverse()
    leftDirections.reverse()
    positions = leftPositions + rightPositions
    directions = leftDirections + rightDirections

    # create RZPs
    rzps = [RZP] * meta.numElements
    for i, currRZP in enumerate(rzps):
        currRZP.worldPosition = positions[i]
        currRZP.worldXdirection = directions[i][0]
        currRZP.worldYdirection = directions[i][1]
        currRZP.worldZdirection = directions[i][2]
        # insert RZP parameters
        root = insertRZP(root, currRZP)

    # xPositions = [pos[0] for pos in positions]
    # zPositions = [pos[2] for pos in positions]
    # plt.scatter(xPositions, zPositions)
    # plt.xlabel('x')
    # plt.ylabel('z')
    # # plt.gca().set_aspect('equal', adjustable='box')
    # plt.show()

    # Plot x directions
    # xDirectionsx = [dir[0][0] for dir in directions]
    # xDirectionsz = [dir[0][2] for dir in directions]
    # plt.scatter(xDirectionsx, xDirectionsz)
    # plt.xlabel('x')
    # plt.ylabel('z')
    # # plt.gca().set_aspect('equal', adjustable='box')
    # plt.show()

    # Plot z directions
    # zDirectionsx = [dir[2][0] for dir in directions]
    # zDirectionsz = [dir[2][2] for dir in directions]
    # plt.scatter(zDirectionsx, zDirectionsz)
    # plt.xlabel('x')
    # plt.ylabel('z')
    # # plt.gca().set_aspect('equal', adjustable='box')
    # plt.show()

    indent(root)
    tmp = ET.tostring(root, encoding='UTF-8', method='xml')
    xmlString = ET.tostring(root, encoding='UTF-8',
                            method='xml').decode('UTF-8')
    xmlToFile(meta.fileName, xmlString)


# main
if __name__ == '__main__':
    main()
