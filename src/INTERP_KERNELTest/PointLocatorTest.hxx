//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
#ifndef __TU_POINTLOCATOR_HXX__
#define __TU_POINTLOCATOR_HXX__

#include <cppunit/extensions/HelperMacros.h>
#include "PointLocator.hxx"

namespace INTERP_TEST
{

  /**
   * \brief Test suite testing some of the low level methods of TransformedTriangle.
   *
   */
  class PointLocatorTest : public CppUnit::TestFixture
  {

    CPPUNIT_TEST_SUITE( PointLocatorTest );
    CPPUNIT_TEST( test_PointLocator );
    CPPUNIT_TEST_SUITE_END();

   
  public:
    void setUp();

    void tearDown();

    // tests
    void test_PointLocator();

  };




}



#endif
