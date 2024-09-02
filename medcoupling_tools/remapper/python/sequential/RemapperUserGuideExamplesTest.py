#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2024  CEA, EDF
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#


from medcoupling import *

#! [UG_Projection_0]
srcCoo=DataArrayDouble([(0,0),(1,0),(3,0),(0,1),(1,1),(3,1)])
src=MEDCouplingUMesh("src",2)
src.setCoords(srcCoo)
src.allocateCells()
src.insertNextCell(NORM_QUAD4,[0,3,4,1])
src.insertNextCell(NORM_QUAD4,[1,4,5,2])
#
trgCoo=DataArrayDouble([(0.5,0.5),(1.5,0.5),(1.5,1.5)])
trg=MEDCouplingUMesh("trg",2)
trg.setCoords(trgCoo)
trg.allocateCells()
trg.insertNextCell(NORM_TRI3,[0,2,1])
#! [UG_Projection_0]
from MEDCouplingRemapper import MEDCouplingRemapper

#! [UG_Projection_1]
rem=MEDCouplingRemapper()
rem.prepare(src,trg,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_1]
#! [UG_Projection_2]
srcF=MEDCouplingFieldDouble(ON_CELLS)
srcF.setMesh(src)
srcF.setArray(DataArrayDouble([3,4]))
srcF.setNature(IntensiveMaximum)
#
trgF=rem.transferField(srcF,-1)
#! [UG_Projection_2]
#! [UG_Projection_3]
rem=MEDCouplingRemapper()
rem.prepare(src,trg,"P0P1")
print(rem.getCrudeMatrix())
#! [UG_Projection_3]
#! [UG_Projection_4]
rem=MEDCouplingRemapper()
rem.prepare(src,trg,"P1P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_4]


#! [UG_Projection_5]
coo=DataArrayDouble([(0.,0.,0.), (1,0,0), (0,1,0)])
src=MEDCouplingUMesh("src",2)
src.setCoords(coo)
src.allocateCells(1)
src.insertNextCell(NORM_TRI3,[0,1,2])
tgt = src.deepCopy()
rem=MEDCouplingRemapper()
rem.prepare(src,tgt,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_5]
#! [UG_Projection_6]
src.translate([0,0,1e-3])
rem.prepare(src,tgt,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_6]
#! [UG_Projection_7]
rem.setBoundingBoxAdjustmentAbs( 1e-3 )
rem.prepare(src,tgt,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_7]

import math

#! [UG_Projection_8]
src.rotate([0,0,0],[0,1,0],math.pi/4)
rem.prepare(src,tgt,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_8]
#! [UG_Projection_9]
rem.setMaxDistance3DSurfIntersect( 0.1 )
rem.prepare(src,tgt,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_9]

#! [UG_Projection_10]
rem.setMaxDistance3DSurfIntersect( -1 ) # switch it off
rem.setMinDotBtwPlane3DSurfIntersect( 0.8 )
rem.prepare(src,tgt,"P0P0")
print(rem.getCrudeMatrix())
#! [UG_Projection_10]
