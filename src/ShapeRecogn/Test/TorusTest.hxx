// Copyright (C) 2007-2024  CEA, EDF
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

#ifndef __TORUSTEST_HXX__
#define __TORUSTEST_HXX__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace MEDCoupling
{
    class ShapeRecognMeshBuilder;
    class Areas;

    class TorusTest : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TorusTest);
        CPPUNIT_TEST(testArea);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() override;
        void tearDown() override;

        void testArea();

    private:
        ShapeRecognMeshBuilder *srMesh = 0;
        const Areas *areas;
    };
};

#endif // __TORUSTEST_HXX__
