// Copyright (C) 2007-2020  CEA/DEN, EDF R&D
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
#ifndef __PLANARINTERSECTORP0P0_TXX__
#define __PLANARINTERSECTORP0P0_TXX__

#include "PlanarIntersectorP0P0.hxx"

namespace INTERP_KERNEL
{
  template<class MyMeshType, class MyMatrix, class ConcreteP0P0Intersector>
  PlanarIntersectorP0P0<MyMeshType,MyMatrix,ConcreteP0P0Intersector>::PlanarIntersectorP0P0(const MyMeshType& meshT, const MyMeshType& meshS,
                                                                                            double dimCaracteristic, double precision, double md3DSurf, double minDot3DSurf, double medianPlane,
                                                                                            bool doRotate, int orientation, int printLevel):
    PlanarIntersector<MyMeshType,MyMatrix>(meshT,meshS,dimCaracteristic,precision,md3DSurf,minDot3DSurf,medianPlane,doRotate,orientation,printLevel)
  {
  }

  template<class MyMeshType, class MyMatrix, class ConcreteP0P0Intersector>
  typename MyMeshType::MyConnType PlanarIntersectorP0P0<MyMeshType,MyMatrix,ConcreteP0P0Intersector>::getNumberOfRowsOfResMatrix() const
  {
    return PlanarIntersector<MyMeshType,MyMatrix>::_meshT.getNumberOfElements();
  }

  template<class MyMeshType, class MyMatrix, class ConcreteP0P0Intersector>
  typename MyMeshType::MyConnType PlanarIntersectorP0P0<MyMeshType,MyMatrix,ConcreteP0P0Intersector>::getNumberOfColsOfResMatrix() const
  {
    return PlanarIntersector<MyMeshType,MyMatrix>::_meshS.getNumberOfElements();
  }

  template<class MyMeshType, class MyMatrix, class ConcreteP0P0Intersector>
  void PlanarIntersectorP0P0<MyMeshType,MyMatrix,ConcreteP0P0Intersector>::intersectCells(ConnType icellT, const std::vector<ConnType>& icellsS, MyMatrix& res)
  {
    ConnType nbNodesT=PlanarIntersector<MyMeshType,MyMatrix>::_connIndexT[icellT+1]-PlanarIntersector<MyMeshType,MyMatrix>::_connIndexT[icellT];
    typename MyMatrix::value_type& resRow=res[icellT];
    for(typename std::vector<ConnType>::const_iterator iter=icellsS.begin();iter!=icellsS.end();iter++)
      {
        ConnType iS=*iter;
        ConnType nbNodesS=PlanarIntersector<MyMeshType,MyMatrix>::_connIndexS[iS+1]-PlanarIntersector<MyMeshType,MyMatrix>::_connIndexS[iS];
        double surf=intersectGeometry(OTT<ConnType,numPol>::indFC(icellT),OTT<ConnType,numPol>::indFC(iS),nbNodesT,nbNodesS);
        surf=PlanarIntersector<MyMeshType,MyMatrix>::getValueRegardingOption(surf);
        if(surf!=0.)
          resRow.insert(std::make_pair(OTT<ConnType,numPol>::indFC(iS),surf));
      }
  }
}

#endif
