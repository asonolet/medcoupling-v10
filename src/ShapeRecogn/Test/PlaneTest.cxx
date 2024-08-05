#include "PlaneTest.hxx"

#include "ShapeRecognMeshBuilder.hxx"
#include "Areas.hxx"
#include "MathOps.hxx"
#include "TestInterpKernelUtils.hxx" // getResourceFile()

using namespace MEDCoupling;

void PlaneTest::setUp()
{
    std::string file = INTERP_TEST::getResourceFile("ShapeRecognPlane.med", 3);
    srMesh = new ShapeRecognMeshBuilder(file);
    srMesh->recognize();
    areas = srMesh->getAreas();
}

void PlaneTest::tearDown()
{
    if (srMesh != 0)
        delete srMesh;
    areas = 0;
}

void PlaneTest::testArea()
{
    CPPUNIT_ASSERT_EQUAL(36, (int)areas->getNumberOfNodes(0));
    CPPUNIT_ASSERT_EQUAL(1, (int)areas->getNumberOfAreas());
    // Normal
    std::array<double, 3> normal = areas->getNormal(0);
    std::array<double, 3> normalRef = {0.781525, 0.310606, -0.541056};
    std::array<double, 3> affinePoint = areas->getAffinePoint(0);
    double proportion0 = normal[0] / normalRef[0];
    double proportion1 = normal[1] / normalRef[1];
    double proportion2 = normal[2] / normalRef[2];
    double proportion3 = MathOps::dot(normal, affinePoint) / MathOps::dot(normalRef, affinePoint);
    // Check proportions between the normal vectors of the two planes
    CPPUNIT_ASSERT_DOUBLES_EQUAL(proportion0, proportion1, 1E-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(proportion1, proportion2, 1E-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(proportion2, proportion3, 1E-6);
    // Check the angle
    double angle = MathOps::computeAngle(normal, normalRef);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, angle, 1E-6);
}