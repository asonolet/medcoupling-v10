// Copyright (C) 2024  CEA, EDF
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#include "CylinderTest.hxx"

#include "ShapeRecognMeshBuilder.hxx"
#include "ShapeRecognMesh.hxx"
#include "Areas.hxx"
#include "MathOps.hxx"
#include "TestInterpKernelUtils.hxx" // getResourceFile()

#include "ShapeRecognTest.hxx"

using namespace MEDCoupling;

void CylinderTest::setUp()
{
    std::string file = INTERP_TEST::getResourceFile("ShapeRecognCylinder.med", 3);
    srMesh = BuildShapeRecognMeshBuilderFromFile(file);
    srMesh->recognize();
    areas = srMesh->getAreas();
}

void CylinderTest::tearDown()
{
    areas = 0;
}

void CylinderTest::testNumberOfAreas()
{
    CPPUNIT_ASSERT_EQUAL(3, (int)areas->getNumberOfAreas());
}

void CylinderTest::testFirstArea()
{
    // primitive type
    CPPUNIT_ASSERT_EQUAL((int)PrimitiveType::Cylinder, (int) areas->getPrimitiveType(0));
    // node ids
    std::vector<mcIdType> nodeIdsRef{
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
        47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
        62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
        77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
        92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104,
        105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
        116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
        127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
        138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
        149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
        160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
        171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
        182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
        193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
        204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
        215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225,
        226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236,
        237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247,
        248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258,
        259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269,
        270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280,
        281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291,
        292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302,
        303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313,
        314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324,
        325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335,
        336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346,
        347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357,
        358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368,
        369};
    std::vector<mcIdType> nodeIds = areas->getNodeIds(0);
    std::sort(nodeIds.begin(), nodeIds.end());
    CPPUNIT_ASSERT_EQUAL(nodeIdsRef.size(), nodeIds.size());
    for (size_t i = 0; i < nodeIds.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(nodeIdsRef[i], nodeIds[i]);
    // radius
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.993494657779537, areas->getRadius(0), 1E-2);
    // axis
    std::array<double, 3> axis = areas->getAxis(0);
    std::array<double, 3> axisRef{7.66631075e-04, -1.59966800e-04, 9.99999693e-01};
    for (size_t i = 0; i < 3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(axisRef[i], axis[i], 1E-4);
    // axis point
    std::array<double, 3> axisPoint = areas->getAxisPoint(0);
    std::array<double, 3> axisPointRef{3.78927537e-03, -2.03409415e-03, 5.03355802e+00};
    for (size_t i = 0; i < 3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(
            axisPointRef[i], axisPoint[i], 1E-2);
}

void CylinderTest::testSecondArea()
{
    // primitive type
    CPPUNIT_ASSERT_EQUAL((int)PrimitiveType::Plane, (int)areas->getPrimitiveType(1));
    // node ids
    std::vector<mcIdType> nodeIdsRef{
        370, 371, 372, 373, 374, 375, 376, 377, 378, 379,
        380, 381, 382, 383, 384, 385, 386, 387, 388, 389,
        390, 391, 392, 393, 394, 395, 396, 397, 398, 399,
        400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
        410, 411, 412, 413, 414, 415, 416, 417, 418, 419,
        420, 421, 422, 423, 424, 425, 426, 427, 428, 429,
        430, 431, 432, 433, 434, 435, 436, 437, 438, 439,
        440, 441, 442};
    std::vector<mcIdType> nodeIds = areas->getNodeIds(1);
    std::sort(nodeIds.begin(), nodeIds.end());
    CPPUNIT_ASSERT_EQUAL(nodeIdsRef.size(), nodeIds.size());
    for (size_t i = 0; i < nodeIds.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(nodeIdsRef[i], nodeIds[i]);
    // normal
    std::array<double, 3> normal = areas->getNormal(1);
    std::array<double, 3> normalRef{0.0, 0.0, 1.0};
    for (size_t i = 0; i < 3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(normalRef[i], normal[i], 1E-6);
}

void CylinderTest::testThirdArea()
{
    // primitive type
    CPPUNIT_ASSERT_EQUAL((int)PrimitiveType::Plane, (int)areas->getPrimitiveType(2));
    // node ids
    std::vector<mcIdType> nodeIdsRef{
        443, 444, 445, 446, 447, 448, 449, 450, 451, 452,
        453, 454, 455, 456, 457, 458, 459, 460, 461, 462,
        463, 464, 465, 466, 467, 468, 469, 470, 471, 472,
        473, 474, 475, 476, 477, 478, 479, 480, 481, 482,
        483, 484, 485, 486, 487, 488, 489, 490, 491, 492,
        493, 494, 495, 496, 497, 498, 499};
    std::vector<mcIdType> nodeIds = areas->getNodeIds(2);
    std::sort(nodeIds.begin(), nodeIds.end());
    CPPUNIT_ASSERT_EQUAL(nodeIdsRef.size(), nodeIds.size());
    for (size_t i = 0; i < nodeIds.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(nodeIdsRef[i], nodeIds[i]);
    // normal
    std::array<double, 3> normal = areas->getNormal(2);
    std::array<double, 3> normalRef{0.0, 0.0, -1.0};
    for (size_t i = 0; i < 3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(normalRef[i], normal[i], 1E-6);
}
