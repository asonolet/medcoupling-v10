// Copyright (C) 2016  CEA/DEN, EDF R&D
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
// Author : Anthony Geay (EDF R&D)

#ifndef __MEDCOUPLINGTRAITS_HXX__
#define __MEDCOUPLINGTRAITS_HXX__

#include "MEDCoupling.hxx"

namespace MEDCoupling
{
  template<class T>
  struct MEDCOUPLING_EXPORT Traits
  {
    typedef T EltType;
  };

  class DataArrayInt;
  class DataArrayFloat;
  class DataArrayDouble;
  class DataArrayChar;
  class DataArrayByte;
  class MEDCouplingFieldDouble;
  class MEDCouplingFieldFloat;
  class MEDCouplingFieldInt;
  class DataArrayIntTuple;
  class DataArrayDoubleTuple;
  
  template<>
  struct MEDCOUPLING_EXPORT Traits<double>
  {
    static const char ArrayTypeName[];
    static const char FieldTypeName[];
    static const char NPYStr[];
    typedef DataArrayDouble ArrayType;
    typedef DataArrayDouble ArrayTypeCh;
    typedef MEDCouplingFieldDouble FieldType;
    typedef DataArrayDoubleTuple ArrayTuple;
  };

  template<>
  struct MEDCOUPLING_EXPORT Traits<float>
  {
    static const char ArrayTypeName[];
    static const char FieldTypeName[];
    static const char NPYStr[];
    typedef DataArrayFloat ArrayType;
    typedef DataArrayFloat ArrayTypeCh;
    typedef MEDCouplingFieldFloat FieldType;
  };
  
  template<>
  struct MEDCOUPLING_EXPORT Traits<int>
  {
    static const char ArrayTypeName[];
    static const char FieldTypeName[];
    typedef DataArrayInt ArrayType;
    typedef DataArrayInt ArrayTypeCh;
    typedef MEDCouplingFieldInt FieldType;
    typedef DataArrayIntTuple ArrayTuple;
  };

  template<>
  struct MEDCOUPLING_EXPORT Traits<char>
  {
    static const char ArrayTypeName[];
    typedef DataArrayByte ArrayTypeCh;
    typedef DataArrayChar ArrayType;
  };
}

#endif
