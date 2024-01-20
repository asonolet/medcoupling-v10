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
// Author : Anthony Geay (CEA/DEN)

#ifndef __CURVEINTERSECTORP0P0_HXX__
#define __CURVEINTERSECTORP0P0_HXX__

#include "CurveIntersector.hxx"

namespace INTERP_KERNEL
{
  template<class MyMeshType, class MyMatrix>
  class CurveIntersectorP0P0 : public CurveIntersector<MyMeshType,MyMatrix>
  {
  public:
    static const int SPACEDIM=MyMeshType::MY_SPACEDIM;
    static const int MESHDIM=MyMeshType::MY_MESHDIM;
    typedef typename MyMeshType::MyConnType ConnType;
    static const NumberingPolicy numPol=MyMeshType::My_numPol;

    CurveIntersectorP0P0(const MyMeshType& meshT, const MyMeshType& meshS,
                         double  precision, double tolerance,
                         double medianLine, int printLevel);
  public:
    ConnType getNumberOfRowsOfResMatrix() const;
    ConnType getNumberOfColsOfResMatrix() const;
    void intersectCells(ConnType icellT,
                        const std::vector<ConnType>& icellsS, MyMatrix& res);
  };
}

#endif
