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
// Author : Anthony Geay (EDF R&D)

#include "MEDCoupling1GTUMesh.txx"
#include "MEDCouplingUMesh.hxx"
#include "MEDCouplingFieldDouble.hxx"
#include "MEDCouplingCMesh.hxx"

#include "SplitterTetra.hxx"
#include "DiameterCalculator.hxx"
#include "OrientationInverter.hxx"
#include "InterpKernelAutoPtr.hxx"
#include "VolSurfUser.txx"

using namespace MEDCoupling;

const int MEDCoupling1SGTUMesh::HEXA8_FACE_PAIRS[6]={0,1,2,4,3,5};

MEDCoupling1GTUMesh::MEDCoupling1GTUMesh():_cm(0)
{
}

MEDCoupling1GTUMesh::MEDCoupling1GTUMesh(const std::string& name, const INTERP_KERNEL::CellModel& cm):_cm(&cm)
{
  setName(name);
}

MEDCoupling1GTUMesh::MEDCoupling1GTUMesh(const MEDCoupling1GTUMesh& other, bool recDeepCpy):MEDCouplingPointSet(other,recDeepCpy),_cm(other._cm)
{
}

MEDCoupling1GTUMesh *MEDCoupling1GTUMesh::New(const std::string& name, INTERP_KERNEL::NormalizedCellType type)
{
  if(type==INTERP_KERNEL::NORM_ERROR)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::New : NORM_ERROR is not a valid type to be used as base geometric type for a mesh !");
  const INTERP_KERNEL::CellModel& cm=INTERP_KERNEL::CellModel::GetCellModel(type);
  if(!cm.isDynamic())
    return MEDCoupling1SGTUMesh::New(name,type);
  else
    return MEDCoupling1DGTUMesh::New(name,type);
}

MEDCoupling1GTUMesh *MEDCoupling1GTUMesh::New(const MEDCouplingUMesh *m)
{
  if(!m)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::New : input mesh is null !");
  std::set<INTERP_KERNEL::NormalizedCellType> gts(m->getAllGeoTypes());
  if(gts.size()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::New : input mesh must have exactly one geometric type !");
  const INTERP_KERNEL::CellModel& cm=INTERP_KERNEL::CellModel::GetCellModel(*gts.begin());
  if(!cm.isDynamic())
    return MEDCoupling1SGTUMesh::New(m);
  else
    return MEDCoupling1DGTUMesh::New(m);
}

const INTERP_KERNEL::CellModel& MEDCoupling1GTUMesh::getCellModel() const
{
  return *_cm;
}

INTERP_KERNEL::NormalizedCellType MEDCoupling1GTUMesh::getCellModelEnum() const
{
  return _cm->getEnum();
}

int MEDCoupling1GTUMesh::getMeshDimension() const
{
  return (int)_cm->getDimension();
}

/*!
 * This method returns a newly allocated array containing cell ids (ascendingly sorted) whose geometric type are equal to type.
 * This method does not throw exception if geometric type \a type is not in \a this.
 * This method throws an INTERP_KERNEL::Exception if meshdimension of \b this is not equal to those of \b type.
 * The coordinates array is not considered here.
 *
 * \param [in] type the geometric type
 * \return cell ids in this having geometric type \a type.
 */
DataArrayIdType *MEDCoupling1GTUMesh::giveCellsWithType(INTERP_KERNEL::NormalizedCellType type) const
{
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  if(type==getCellModelEnum())
    ret->alloc(getNumberOfCells(),1);
  else
    ret->alloc(0,1);
  ret->iota();
  return ret.retn();
}

/*!
 * Returns nb of cells having the geometric type \a type. No throw if no cells in \a this has the geometric type \a type.
 */
mcIdType MEDCoupling1GTUMesh::getNumberOfCellsWithType(INTERP_KERNEL::NormalizedCellType type) const
{
  return type==getCellModelEnum()?getNumberOfCells():0;
}

/*!
 * Returns a type of a cell by its id.
 *  \param [in] cellId - the id of the cell of interest.
 *  \return INTERP_KERNEL::NormalizedCellType - enumeration item describing the cell type.
 *  \throw If \a cellId is invalid. Valid range is [0, \a this->getNumberOfCells() ).
 */
INTERP_KERNEL::NormalizedCellType MEDCoupling1GTUMesh::getTypeOfCell(mcIdType cellId) const
{
  if(cellId<getNumberOfCells())
    return getCellModelEnum();
  std::ostringstream oss; oss << "MEDCoupling1GTUMesh::getTypeOfCell : Requesting type of cell #" << cellId << " but it should be in [0," << getNumberOfCells() << ") !";
  throw INTERP_KERNEL::Exception(oss.str().c_str());
}

/*!
 * Returns a set of all cell types available in \a this mesh.
 * \return std::set<INTERP_KERNEL::NormalizedCellType> - the set of cell types.
 * \warning this method does not throw any exception even if \a this is not defined.
 */
std::set<INTERP_KERNEL::NormalizedCellType> MEDCoupling1GTUMesh::getAllGeoTypes() const
{
  std::set<INTERP_KERNEL::NormalizedCellType> ret;
  ret.insert(getCellModelEnum());
  return ret;
}

/*!
 * This method expects that \a this is sorted by types. If not an exception will be thrown.
 * This method returns in the same format as code (see MEDCouplingUMesh::checkTypeConsistencyAndContig or MEDCouplingUMesh::splitProfilePerType) how
 * \a this is composed in cell types.
 * The returned array is of size 3*n where n is the number of different types present in \a this. 
 * For every k in [0,n] ret[3*k+2]==-1 because it has no sense here. 
 * This parameter is kept only for compatibility with other method listed above.
 */
std::vector<mcIdType> MEDCoupling1GTUMesh::getDistributionOfTypes() const
{
  std::vector<mcIdType> ret(3);
  ret[0]=ToIdType(getCellModelEnum()); ret[1]=getNumberOfCells(); ret[2]=-1;
  return ret;
}

/*!
 * This method is the opposite of MEDCouplingUMesh::checkTypeConsistencyAndContig method. Given a list of cells in \a profile it returns a list of sub-profiles sorted by geo type.
 * The result is put in the array \a idsPerType. In the returned parameter \a code, foreach i \a code[3*i+2] refers (if different from -1) to a location into the \a idsPerType.
 * This method has 1 input \a profile and 3 outputs \a code \a idsInPflPerType and \a idsPerType.
 * 
 * \param [out] code is a vector of size 3*n where n is the number of different geometric type in \a this \b reduced to the profile \a profile. \a code has exactly the same semantic than in MEDCouplingUMesh::checkTypeConsistencyAndContig method.
 * \param [out] idsInPflPerType is a vector of size of different geometric type in the subpart defined by \a profile of \a this ( equal to \a code.size()/3). For each i,
 *              \a idsInPflPerType[i] stores the tuple ids in \a profile that correspond to the geometric type code[3*i+0]
 * \param [out] idsPerType is a vector of size of different sub profiles needed to be defined to represent the profile \a profile for a given geometric type.
 *              This vector can be empty in case of all geometric type cells are fully covered in ascending in the given input \a profile.
 * 
 * \warning for performance reasons no deep copy will be performed, if \a profile can been used as this in output parameters \a idsInPflPerType and \a idsPerType.
 *
 * \throw if \a profile has not exactly one component. It throws too, if \a profile contains some values not in [0,getNumberOfCells()) or if \a this is not fully defined
 *
 *  \b Example1: <br>
 *          - Before \a this has 3 cells \a profile contains [0,1,2]
 *          - After \a code contains [NORM_...,nbCells,-1], \a idsInPflPerType [[0,1,2]] and \a idsPerType is empty <br>
 * 
 *  \b Example2: <br>
 *          - Before \a this has 3 cells \a profile contains [1,2]
 *          - After \a code contains [NORM_...,nbCells,0], \a idsInPflPerType [[0,1]] and \a idsPerType is [[1,2]] <br>

 */
void MEDCoupling1GTUMesh::splitProfilePerType(const DataArrayIdType *profile, std::vector<mcIdType>& code, std::vector<DataArrayIdType *>& idsInPflPerType, std::vector<DataArrayIdType *>& idsPerType, bool smartPflKiller) const
{
  if(!profile)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::splitProfilePerType : input profile is NULL !");
  if(profile->getNumberOfComponents()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::splitProfilePerType : input profile should have exactly one component !");
  mcIdType nbTuples=profile->getNumberOfTuples(),nbOfCells=getNumberOfCells();
  code.resize(3); idsInPflPerType.resize(1);
  code[0]=ToIdType(getCellModelEnum()); code[1]=nbTuples;
  idsInPflPerType.resize(1);
  if(smartPflKiller && profile->isIota(nbOfCells))
    {
      code[2]=-1;
      idsInPflPerType[0]=const_cast<DataArrayIdType *>(profile); idsInPflPerType[0]->incrRef();
      idsPerType.clear();
      return ;
    }
  code[2]=0;
  profile->checkAllIdsInRange(0,nbOfCells);
  idsPerType.resize(1);
  idsPerType[0]=const_cast<DataArrayIdType *>(profile); idsPerType[0]->incrRef();
  idsInPflPerType[0]=DataArrayIdType::Range(0,nbTuples,1);
}

/*!
 * This method tries to minimize at most the number of deep copy.
 * So if \a idsPerType is not empty it can be returned directly (without copy, but with ref count incremented) in return.
 * 
 * \sa MEDCouplingUMesh::checkTypeConsistencyAndContig
 */
DataArrayIdType *MEDCoupling1GTUMesh::checkTypeConsistencyAndContig(const std::vector<mcIdType>& code, const std::vector<const DataArrayIdType *>& idsPerType) const
{
  mcIdType nbOfCells=getNumberOfCells();
  if(code.size()!=3)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : invalid input code should be exactly of size 3 !");
  if(code[0]!=ToIdType(getCellModelEnum()))
    {
      std::ostringstream oss; oss << "MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : Mismatch of geometric type ! Asking for " << code[0] << " whereas the geometric type is \a this is " << getCellModelEnum() << " (" << _cm->getRepr() << ") !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
  if(code[2]==-1)
    {
      if(code[1]==nbOfCells)
        return 0;
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : mismatch between the number of cells in this (" << nbOfCells << ") and the number of non profile (" << code[1] << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  if(code[2]!=0)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : single geo type mesh ! 0 or -1 is expected at pos #2 of input code !");
  if(idsPerType.size()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : input code points to DataArrayIdType #0 whereas the size of idsPerType is not equal to 1 !");
  const DataArrayIdType *pfl=idsPerType[0];
  if(!pfl)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : the input code points to a NULL DataArrayIdType at rank 0 !");
  if(pfl->getNumberOfComponents()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::checkTypeConsistencyAndContig : input profile should have exactly one component !");
  pfl->checkAllIdsInRange(0,nbOfCells);
  pfl->incrRef();
  return const_cast<DataArrayIdType *>(pfl);
}

void MEDCoupling1GTUMesh::writeVTKLL(std::ostream& ofs, const std::string& cellData, const std::string& pointData, DataArrayByte *byteData) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  m->writeVTKLL(ofs,cellData,pointData,byteData);
}

std::string MEDCoupling1GTUMesh::getVTKDataSetType() const
{
  return std::string("UnstructuredGrid");
}

std::string MEDCoupling1GTUMesh::getVTKFileExtension() const
{
  return std::string("vtu");
}

std::size_t MEDCoupling1GTUMesh::getHeapMemorySizeWithoutChildren() const
{
  return MEDCouplingPointSet::getHeapMemorySizeWithoutChildren();
}

bool MEDCoupling1GTUMesh::isEqualIfNotWhy(const MEDCouplingMesh *other, double prec, std::string& reason) const
{
  if(!MEDCouplingPointSet::isEqualIfNotWhy(other,prec,reason))
    return false;
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::isEqualIfNotWhy : input other pointer is null !");
  const MEDCoupling1GTUMesh *otherC=dynamic_cast<const MEDCoupling1GTUMesh *>(other);
  if(!otherC)
    {
      reason="mesh given in input is not castable in MEDCouplingSGTUMesh !";
      return false;
    }
  if(_cm!=otherC->_cm)
    {
      reason="mismatch in geometric type !";
      return false;
    }
  return true;
}

bool MEDCoupling1GTUMesh::isEqualWithoutConsideringStr(const MEDCouplingMesh *other, double prec) const
{
  if(!MEDCouplingPointSet::isEqualWithoutConsideringStr(other,prec))
    return false;
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::isEqualWithoutConsideringStr : input other pointer is null !");
  const MEDCoupling1GTUMesh *otherC=dynamic_cast<const MEDCoupling1GTUMesh *>(other);
  if(!otherC)
    return false;
  if(_cm!=otherC->_cm)
    return false;
  return true;
}

void MEDCoupling1GTUMesh::checkConsistencyLight() const
{
  MEDCouplingPointSet::checkConsistencyLight();
}

DataArrayDouble *MEDCoupling1GTUMesh::computeCellCenterOfMass() const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  MCAuto<DataArrayDouble> ret=m->computeCellCenterOfMass();
  return ret.retn();
}

MEDCouplingFieldDouble *MEDCoupling1GTUMesh::getMeasureField(bool isAbs) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  MCAuto<MEDCouplingFieldDouble> ret=m->getMeasureField(isAbs);
  ret->setMesh(this);
  return ret.retn();
}

MEDCouplingFieldDouble *MEDCoupling1GTUMesh::getMeasureFieldOnNode(bool isAbs) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  MCAuto<MEDCouplingFieldDouble> ret=m->getMeasureFieldOnNode(isAbs);
  ret->setMesh(this);
  return ret.retn();
}

/*!
 * to improve perf !
 */
mcIdType MEDCoupling1GTUMesh::getCellContainingPoint(const double *pos, double eps) const
{
  MCAuto<MEDCouplingUMesh> m(buildUnstructured());
  return m->getCellContainingPoint(pos,eps);
}

/*!
 * to improve perf !
 */
void MEDCoupling1GTUMesh::getCellsContainingPoint(const double *pos, double eps, std::vector<mcIdType>& elts) const
{
  MCAuto<MEDCouplingUMesh> m(buildUnstructured());
  return m->getCellsContainingPoint(pos,eps,elts);
}

/*!
 * to improve perf !
 */
void MEDCoupling1GTUMesh::getCellsContainingPoints(const double *pos, mcIdType nbOfPoints, double eps, MCAuto<DataArrayIdType>& elts, MCAuto<DataArrayIdType>& eltsIndex) const
{// See EDF29571
  MCAuto<MEDCouplingUMesh> m(buildUnstructured());
  m->getCellsContainingPoints(pos,nbOfPoints,eps,elts,eltsIndex);
}

MEDCouplingFieldDouble *MEDCoupling1GTUMesh::buildOrthogonalField() const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  MCAuto<MEDCouplingFieldDouble> ret=m->buildOrthogonalField();
  ret->setMesh(this);
  return ret.retn();
}

DataArrayIdType *MEDCoupling1GTUMesh::getCellsInBoundingBox(const double *bbox, double eps) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  return m->getCellsInBoundingBox(bbox,eps);
}

DataArrayIdType *MEDCoupling1GTUMesh::getCellsInBoundingBox(const INTERP_KERNEL::DirectedBoundingBox& bbox, double eps)
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  return m->getCellsInBoundingBox(bbox,eps);
}

MEDCouplingPointSet *MEDCoupling1GTUMesh::buildFacePartOfMySelfNode(const mcIdType *start, const mcIdType *end, bool fullyIn) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  return m->buildFacePartOfMySelfNode(start,end,fullyIn);
}

DataArrayIdType *MEDCoupling1GTUMesh::findBoundaryNodes() const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  return m->findBoundaryNodes();
}

MEDCouplingPointSet *MEDCoupling1GTUMesh::buildBoundaryMesh(bool keepCoords) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  return m->buildBoundaryMesh(keepCoords);
}

void MEDCoupling1GTUMesh::findCommonCells(int compType, mcIdType startCellId, DataArrayIdType *& commonCellsArr, DataArrayIdType *& commonCellsIArr) const
{
  MCAuto<MEDCouplingUMesh> m=buildUnstructured();
  m->findCommonCells(compType,startCellId,commonCellsArr,commonCellsIArr);
}

mcIdType MEDCoupling1GTUMesh::getNodalConnectivityLength() const
{
  const DataArrayIdType *c1(getNodalConnectivity());
  if(!c1)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::getNodalConnectivityLength : no connectivity set !");
  if(c1->getNumberOfComponents()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::getNodalConnectivityLength : Nodal connectivity array set must have exactly one component !");
  if(!c1->isAllocated())
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::getNodalConnectivityLength : Nodal connectivity array must be allocated !");
  return c1->getNumberOfTuples();
}

/*!
 * This method aggregates all the meshes in \a parts to put them in a single unstructured mesh (those returned).
 * The order of cells is the returned instance is those in the order of instances in \a parts.
 *
 * \param [in] parts - all not null parts of single geo type meshes to be aggreagated having the same mesh dimension and same coordinates.
 * \return MEDCouplingUMesh * - new object to be dealt by the caller.
 *
 * \throw If one element is null in \a parts.
 * \throw If not all the parts do not have the same mesh dimension.
 * \throw If not all the parts do not share the same coordinates.
 * \throw If not all the parts have their connectivity set properly.
 * \throw If \a parts is empty.
 */
MEDCouplingUMesh *MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh(const std::vector< const MEDCoupling1GTUMesh *>& parts)
{
  if(parts.empty())
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh : input parts vector is empty !");
  const MEDCoupling1GTUMesh *firstPart(parts[0]);
  if(!firstPart)
    throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh : the first instance in input parts is null !");
  const DataArrayDouble *coords(firstPart->getCoords());
  int meshDim(firstPart->getMeshDimension());
  MCAuto<MEDCouplingUMesh> ret(MEDCouplingUMesh::New(firstPart->getName(),meshDim)); ret->setDescription(firstPart->getDescription());
  ret->setCoords(coords);
  mcIdType nbOfCells(0),connSize(0);
  for(std::vector< const MEDCoupling1GTUMesh *>::const_iterator it=parts.begin();it!=parts.end();it++)
    {
      if(!(*it))
        throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh : presence of null pointer in input vector !");
      if((*it)->getMeshDimension()!=meshDim)
        throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh : all the instances in input vector must have same mesh dimension !");
      if((*it)->getCoords()!=coords)
        throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh : all the instances must share the same coordinates pointer !");
      nbOfCells+=(*it)->getNumberOfCells();
      connSize+=(*it)->getNodalConnectivityLength();
    }
  MCAuto<DataArrayIdType> conn(DataArrayIdType::New()),connI(DataArrayIdType::New());
  connI->alloc(nbOfCells+1,1); conn->alloc(connSize+nbOfCells,1);
  mcIdType *c(conn->getPointer()),*ci(connI->getPointer()); *ci=0;
  for(std::vector< const MEDCoupling1GTUMesh *>::const_iterator it=parts.begin();it!=parts.end();it++)
    {
      mcIdType curNbCells=(*it)->getNumberOfCells();
      mcIdType geoType(ToIdType((*it)->getCellModelEnum()));
      const mcIdType *cinPtr((*it)->getNodalConnectivity()->begin());
      const MEDCoupling1SGTUMesh *ps(dynamic_cast<const MEDCoupling1SGTUMesh *>(*it));
      const MEDCoupling1DGTUMesh *pd(dynamic_cast<const MEDCoupling1DGTUMesh *>(*it));
      if(ps && !pd)
        {
          mcIdType nNodesPerCell(ps->getNumberOfNodesPerCell());
          for(int i=0;i<curNbCells;i++,ci++,cinPtr+=nNodesPerCell)
            {
              *c++=geoType;
              c=std::copy(cinPtr,cinPtr+nNodesPerCell,c);
              ci[1]=ci[0]+nNodesPerCell+1;
            }
        }
      else if(!ps && pd)
        {
          const mcIdType *ciinPtr(pd->getNodalConnectivityIndex()->begin());
          for(int i=0;i<curNbCells;i++,ci++,ciinPtr++)
            {
              *c++=geoType;
              c=std::copy(cinPtr+ciinPtr[0],cinPtr+ciinPtr[1],c);
              ci[1]=ci[0]+ciinPtr[1]-ciinPtr[0]+1;
            }
        }
      else
        throw INTERP_KERNEL::Exception("MEDCoupling1GTUMesh::AggregateOnSameCoordsToUMesh : presence of instance which type is not in [MEDCoupling1SGTUMesh,MEDCoupling1DGTUMesh] !");
    }
  ret->setConnectivity(conn,connI,true);
  return ret.retn();
}

//==

MEDCoupling1SGTUMesh::MEDCoupling1SGTUMesh(const MEDCoupling1SGTUMesh& other, bool recDeepCpy):MEDCoupling1GTUMesh(other,recDeepCpy),_conn(other._conn)
{
  if(recDeepCpy)
    {
      const DataArrayIdType *c(other._conn);
      if(c)
        _conn=c->deepCopy();
    }
}

MEDCoupling1SGTUMesh::MEDCoupling1SGTUMesh(const std::string& name, const INTERP_KERNEL::CellModel& cm):MEDCoupling1GTUMesh(name,cm)
{
}

MEDCoupling1SGTUMesh::MEDCoupling1SGTUMesh()
{
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::New()
{
  return new MEDCoupling1SGTUMesh;
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::New(const std::string& name, INTERP_KERNEL::NormalizedCellType type)
{
  if(type==INTERP_KERNEL::NORM_ERROR)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::New : NORM_ERROR is not a valid type to be used as base geometric type for a mesh !");
  const INTERP_KERNEL::CellModel& cm=INTERP_KERNEL::CellModel::GetCellModel(type);
  if(cm.isDynamic())
    {
      std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::New : the input geometric type " << cm.getRepr() << " is dynamic ! Only static types are allowed here !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
  return new MEDCoupling1SGTUMesh(name,cm);
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::New(const MEDCouplingUMesh *m)
{
  if(!m)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::New : input mesh is null !");
  std::set<INTERP_KERNEL::NormalizedCellType> gts(m->getAllGeoTypes());
  if(gts.size()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::New : input mesh must have exactly one geometric type !");
  mcIdType geoType(ToIdType(*gts.begin()));
  MCAuto<MEDCoupling1SGTUMesh> ret(MEDCoupling1SGTUMesh::New(m->getName(),*gts.begin()));
  ret->setCoords(m->getCoords()); ret->setDescription(m->getDescription());
  mcIdType nbCells=m->getNumberOfCells();
  mcIdType nbOfNodesPerCell(ret->getNumberOfNodesPerCell());
  MCAuto<DataArrayIdType> conn(DataArrayIdType::New()); conn->alloc(nbCells*nbOfNodesPerCell,1);
  mcIdType *c(conn->getPointer());
  const mcIdType *cin(m->getNodalConnectivity()->begin()),*ciin(m->getNodalConnectivityIndex()->begin());
  for(mcIdType i=0;i<nbCells;i++,ciin++)
    {
      if(cin[ciin[0]]==geoType)
        {
          if(ciin[1]-ciin[0]==nbOfNodesPerCell+1)
            c=std::copy(cin+ciin[0]+1,cin+ciin[1],c);
          else
            {
              std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::New(const MEDCouplingUMesh *m) : something is wrong in the input mesh at cell #" << i << " ! The size of cell is not those expected (" << nbOfNodesPerCell << ") !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::New(const MEDCouplingUMesh *m) : something is wrong in the input mesh at cell #" << i << " ! The geometric type is not those expected !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  ret->setNodalConnectivity(conn);
  try
  { ret->copyTinyInfoFrom(m); }
  catch(INTERP_KERNEL::Exception&) { }
  return ret.retn();
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::clone(bool recDeepCpy) const
{
  return new MEDCoupling1SGTUMesh(*this,recDeepCpy);
}

/*!
 * This method behaves mostly like MEDCoupling1SGTUMesh::deepCopy method, except that only nodal connectivity arrays are deeply copied.
 * The coordinates are shared between \a this and the returned instance.
 * 
 * \return MEDCoupling1SGTUMesh * - A new object instance holding the copy of \a this (deep for connectivity, shallow for coordiantes)
 * \sa MEDCoupling1SGTUMesh::deepCopy
 */
MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::deepCopyConnectivityOnly() const
{
  checkConsistencyLight();
  MCAuto<MEDCoupling1SGTUMesh> ret(clone(false));
  MCAuto<DataArrayIdType> c(_conn->deepCopy());
  ret->setNodalConnectivity(c);
  return ret.retn();
}

void MEDCoupling1SGTUMesh::shallowCopyConnectivityFrom(const MEDCouplingPointSet *other)
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::shallowCopyConnectivityFrom : input pointer is null !");
  const MEDCoupling1SGTUMesh *otherC=dynamic_cast<const MEDCoupling1SGTUMesh *>(other);
  if(!otherC)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::shallowCopyConnectivityFrom : input pointer is not an MEDCoupling1SGTUMesh instance !");
  setNodalConnectivity(otherC->getNodalConnectivity());
}

void MEDCoupling1SGTUMesh::updateTime() const
{
  MEDCoupling1GTUMesh::updateTime();
  const DataArrayIdType *c(_conn);
  if(c)
    updateTimeWith(*c);
}

std::size_t MEDCoupling1SGTUMesh::getHeapMemorySizeWithoutChildren() const
{
  return MEDCoupling1GTUMesh::getHeapMemorySizeWithoutChildren();
}

std::vector<const BigMemoryObject *> MEDCoupling1SGTUMesh::getDirectChildrenWithNull() const
{
  std::vector<const BigMemoryObject *> ret(MEDCoupling1GTUMesh::getDirectChildrenWithNull());
  ret.push_back((const DataArrayIdType *)_conn);
  return ret;
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::deepCopy() const
{
  return clone(true);
}

bool MEDCoupling1SGTUMesh::isEqualIfNotWhy(const MEDCouplingMesh *other, double prec, std::string& reason) const
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::isEqualIfNotWhy : input other pointer is null !");
  std::ostringstream oss; oss.precision(15);
  const MEDCoupling1SGTUMesh *otherC=dynamic_cast<const MEDCoupling1SGTUMesh *>(other);
  if(!otherC)
    {
      reason="mesh given in input is not castable in MEDCoupling1SGTUMesh !";
      return false;
    }
  if(!MEDCoupling1GTUMesh::isEqualIfNotWhy(other,prec,reason))
    return false;
  const DataArrayIdType *c1(_conn),*c2(otherC->_conn);
  if(c1==c2)
    return true;
  if(!c1 || !c2)
    {
      reason="in connectivity of single static geometric type exactly one among this and other is null !";
      return false;
    }
  if(!c1->isEqualIfNotWhy(*c2,reason))
    {
      reason.insert(0,"Nodal connectivity DataArrayIdType differ : ");
      return false;
    }
  return true;
}

bool MEDCoupling1SGTUMesh::isEqualWithoutConsideringStr(const MEDCouplingMesh *other, double prec) const
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::isEqualWithoutConsideringStr : input other pointer is null !");
  const MEDCoupling1SGTUMesh *otherC=dynamic_cast<const MEDCoupling1SGTUMesh *>(other);
  if(!otherC)
    return false;
  if(!MEDCoupling1GTUMesh::isEqualWithoutConsideringStr(other,prec))
    return false;
  const DataArrayIdType *c1(_conn),*c2(otherC->_conn);
  if(c1==c2)
    return true;
  if(!c1 || !c2)
    return false;
  if(!c1->isEqualWithoutConsideringStr(*c2))
    return false;
  return true;
}

void MEDCoupling1SGTUMesh::checkConsistencyOfConnectivity() const
{
  const DataArrayIdType *c1(_conn);
  if(c1)
    {
      if(c1->getNumberOfComponents()!=1)
        throw INTERP_KERNEL::Exception("Nodal connectivity array is expected to be with number of components set to one !");
      if(c1->getInfoOnComponent(0)!="")
        throw INTERP_KERNEL::Exception("Nodal connectivity array is expected to have no info on its single component !");
      c1->checkAllocated();
    }
  else
    throw INTERP_KERNEL::Exception("Nodal connectivity array not defined !");
}

void MEDCoupling1SGTUMesh::checkConsistencyLight() const
{
  MEDCouplingPointSet::checkConsistencyLight();
  checkConsistencyOfConnectivity();
}

void MEDCoupling1SGTUMesh::checkConsistency(double eps) const
{
  checkConsistencyLight();
  const DataArrayIdType *c1(_conn);
  mcIdType nbOfTuples(c1->getNumberOfTuples());
  mcIdType nbOfNodesPerCell=_cm->getNumberOfNodes();
  if(nbOfTuples%nbOfNodesPerCell!=0)
    {
      std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::checkConsistency : the nb of tuples in conn is " << nbOfTuples << " and number of nodes per cell is " << nbOfNodesPerCell << ". But " << nbOfTuples << "%" << nbOfNodesPerCell << " !=0 !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
  mcIdType nbOfNodes=getNumberOfNodes();
  mcIdType nbOfCells=nbOfTuples/nbOfNodesPerCell;
  const mcIdType *w(c1->begin());
  for(mcIdType i=0;i<nbOfCells;i++)
    for(int j=0;j<nbOfNodesPerCell;j++,w++)
      {
        if(*w<0 || *w>=nbOfNodes)
          {
            std::ostringstream oss; oss << "At node #" << j << " of  cell #" << i << ", is equal to " << *w << " must be in [0," << nbOfNodes << ") !";
            throw INTERP_KERNEL::Exception(oss.str().c_str());
          }
      }
}

mcIdType MEDCoupling1SGTUMesh::getNumberOfCells() const
{
  mcIdType nbOfTuples(getNodalConnectivityLength());
  mcIdType nbOfNodesPerCell(getNumberOfNodesPerCell());
  if(nbOfTuples%nbOfNodesPerCell!=0)
    {
      std::ostringstream oss; oss << "MEDCoupling1SGTUMesh:getNumberOfCells: : the nb of tuples in conn is " << nbOfTuples << " and number of nodes per cell is " << nbOfNodesPerCell << ". But " << nbOfTuples << "%" << nbOfNodesPerCell << " !=0 !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
  return nbOfTuples/nbOfNodesPerCell;
}

mcIdType MEDCoupling1SGTUMesh::getNumberOfNodesInCell(mcIdType cellId) const
{
  return getNumberOfNodesPerCell();
}

mcIdType MEDCoupling1SGTUMesh::getNumberOfNodesPerCell() const
{
  checkNonDynamicGeoType();
  return _cm->getNumberOfNodes();
}

DataArrayIdType *MEDCoupling1SGTUMesh::computeNbOfNodesPerCell() const
{
  checkNonDynamicGeoType();
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(getNumberOfCells(),1);
  ret->fillWithValue(_cm->getNumberOfNodes());
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::computeNbOfFacesPerCell() const
{
  checkNonDynamicGeoType();
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(getNumberOfCells(),1);
  ret->fillWithValue(ToIdType(_cm->getNumberOfSons()));
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::computeEffectiveNbOfNodesPerCell() const
{
  checkNonDynamicGeoType();
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  mcIdType nbCells=getNumberOfCells();
  ret->alloc(nbCells,1);
  mcIdType *retPtr(ret->getPointer());
  mcIdType nbNodesPerCell(getNumberOfNodesPerCell());
  const mcIdType *conn(_conn->begin());
  for(mcIdType i=0;i<nbCells;i++,conn+=nbNodesPerCell,retPtr++)
    {
      std::set<mcIdType> s(conn,conn+nbNodesPerCell);
      *retPtr=ToIdType(s.size());
    }
  return ret.retn();
}

void MEDCoupling1SGTUMesh::getNodeIdsOfCell(mcIdType cellId, std::vector<mcIdType>& conn) const
{
  mcIdType sz=getNumberOfNodesPerCell();
  conn.resize(sz);
  if(cellId<getNumberOfCells())
    std::copy(_conn->begin()+cellId*sz,_conn->begin()+(cellId+1)*sz,conn.begin());
  else
    {
      std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::getNodeIdsOfCell : request for cellId #" << cellId << " must be in [0," << getNumberOfCells() << ") !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
}

void MEDCoupling1SGTUMesh::checkNonDynamicGeoType() const
{
  if(_cm->isDynamic())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkNonDynamicGeoType : internal error ! the internal geo type is dynamic ! should be static !");
}

std::string MEDCoupling1SGTUMesh::simpleRepr() const
{
  static const char msg0[]="No coordinates specified !";
  std::ostringstream ret;
  if(!_cm)
    {
      ret << "No geometric type specified" << std::endl;
      return ret.str();
    }
  ret << "Single static geometic type (" << _cm->getRepr() << ") unstructured mesh with name : \"" << getName() << "\"\n";
  ret << "Description of mesh : \"" << getDescription() << "\"\n";
  int tmpp1,tmpp2;
  double tt=getTime(tmpp1,tmpp2);
  ret << "Time attached to the mesh [unit] : " << tt << " [" << getTimeUnit() << "]\n";
  ret << "Iteration : " << tmpp1  << " Order : " << tmpp2 << "\n";
  ret << "Mesh dimension : " << getMeshDimension() << "\nSpace dimension : ";
  if(_coords!=0)
    {
      const int spaceDim=getSpaceDimension();
      ret << spaceDim << "\nInfo attached on space dimension : ";
      for(int i=0;i<spaceDim;i++)
        ret << "\"" << _coords->getInfoOnComponent(i) << "\" ";
      ret << "\n";
    }
  else
    ret << msg0 << "\n";
  ret << "Number of nodes : ";
  if(_coords!=0)
    ret << getNumberOfNodes() << "\n";
  else
    ret << msg0 << "\n";
  ret << "Number of cells : ";
  if((const DataArrayIdType *)_conn)
    {
      if(_conn->isAllocated())
        {
          if(_conn->getNumberOfComponents()==1)
            ret << getNumberOfCells() << "\n";
          else
            ret << "Nodal connectivity array specified and allocated but with not exactly one component !" << "\n";
        }
      else
        ret << "Nodal connectivity array specified but not allocated !" << "\n";
    }
  else
    ret << "No connectivity specified !" << "\n";
  ret << "Cell type : " << _cm->getRepr() << "\n";
  return ret.str();
}

std::string MEDCoupling1SGTUMesh::advancedRepr() const
{
  std::ostringstream ret;
  ret << simpleRepr();
  ret << "\nCoordinates array : \n___________________\n\n";
  if(_coords)
    _coords->reprWithoutNameStream(ret);
  else
    ret << "No array set !\n";
  ret << "\n\nConnectivity array : \n____________________\n\n";
  //
  if((const DataArrayIdType *)_conn)
    {
      if(_conn->isAllocated())
        {
          if(_conn->getNumberOfComponents()==1)
            {
              mcIdType nbOfCells=getNumberOfCells();
              mcIdType sz=getNumberOfNodesPerCell();
              const mcIdType *connPtr=_conn->begin();
              for(mcIdType i=0;i<nbOfCells;i++,connPtr+=sz)
                {
                  ret << "Cell #" << i << " : ";
                  std::copy(connPtr,connPtr+sz,std::ostream_iterator<mcIdType>(ret," "));
                  ret << "\n";
                }
            }
          else
            ret << "Nodal connectivity array specified and allocated but with not exactly one component !" << "\n";
        }
      else
        ret << "Nodal connectivity array specified but not allocated !" << "\n";
    }
  else
    ret << "No connectivity specified !" << "\n";
  return ret.str();
}

DataArrayDouble *MEDCoupling1SGTUMesh::computeIsoBarycenterOfNodesPerCell() const
{
  MCAuto<DataArrayDouble> ret=DataArrayDouble::New();
  int spaceDim=getSpaceDimension();
  mcIdType nbOfCells=getNumberOfCells();//checkConsistencyLight()
  mcIdType nbOfNodes=getNumberOfNodes();
  ret->alloc(nbOfCells,spaceDim);
  double *ptToFill=ret->getPointer();
  const double *coor=_coords->begin();
  const mcIdType *nodal=_conn->begin();
  mcIdType sz=getNumberOfNodesPerCell();
  double coeff=1./FromIdType<double>(sz);
  for(mcIdType i=0;i<nbOfCells;i++,ptToFill+=spaceDim)
    {
      std::fill(ptToFill,ptToFill+spaceDim,0.);
      for(mcIdType j=0;j<sz;j++,nodal++)
        if(*nodal>=0 && *nodal<nbOfNodes)
          std::transform(coor+spaceDim*nodal[0],coor+spaceDim*(nodal[0]+1),ptToFill,ptToFill,std::plus<double>());
        else
          {
            std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::computeIsoBarycenterOfNodesPerCell : on cell #" << i << " presence of nodeId #" << *nodal << " should be in [0," <<   nbOfNodes << ") !";
            throw INTERP_KERNEL::Exception(oss.str().c_str());
          }
      std::transform(ptToFill,ptToFill+spaceDim,ptToFill,std::bind(std::multiplies<double>(),std::placeholders::_1,coeff));
    }
  return ret.retn();
}

void MEDCoupling1SGTUMesh::renumberCells(const mcIdType *old2NewBg, bool check)
{
  mcIdType nbCells=getNumberOfCells();
  MCAuto<DataArrayIdType> o2n=DataArrayIdType::New();
  o2n->useArray(old2NewBg,false,DeallocType::C_DEALLOC,nbCells,1);
  if(check)
    o2n=o2n->checkAndPreparePermutation();
  //
  const mcIdType *conn=_conn->begin();
  MCAuto<DataArrayIdType> n2o=o2n->invertArrayO2N2N2O(nbCells);
  const mcIdType *n2oPtr=n2o->begin();
  MCAuto<DataArrayIdType> newConn=DataArrayIdType::New();
  newConn->alloc(_conn->getNumberOfTuples(),1);
  newConn->copyStringInfoFrom(*_conn);
  mcIdType sz=getNumberOfNodesPerCell();
  //
  mcIdType *newC=newConn->getPointer();
  for(mcIdType i=0;i<nbCells;i++,newC+=sz)
    {
      mcIdType pos=n2oPtr[i];
      std::copy(conn+pos*sz,conn+(pos+1)*sz,newC);
    }
  _conn=newConn;
}

/*!
 * Keeps from \a this only cells which constituing point id are in the ids specified by [\a begin,\a end).
 * The resulting cell ids are stored at the end of the 'cellIdsKept' parameter.
 * Parameter \a fullyIn specifies if a cell that has part of its nodes in ids array is kept or not.
 * If \a fullyIn is true only cells whose ids are \b fully contained in [\a begin,\a end) tab will be kept.
 *
 * \param [in] begin input start of array of node ids.
 * \param [in] end input end of array of node ids.
 * \param [in] fullyIn input that specifies if all node ids must be in [\a begin,\a end) array to consider cell to be in.
 * \param [in,out] cellIdsKeptArr array where all candidate cell ids are put at the end.
 */
void MEDCoupling1SGTUMesh::fillCellIdsToKeepFromNodeIds(const mcIdType *begin, const mcIdType *end, bool fullyIn, DataArrayIdType *&cellIdsKeptArr) const
{
  mcIdType nbOfCells=getNumberOfCells();
  MCAuto<DataArrayIdType> cellIdsKept=DataArrayIdType::New(); cellIdsKept->alloc(0,1);
  mcIdType tmp=-1;
  mcIdType sz=_conn->getMaxValue(tmp); sz=std::max(sz,ToIdType(0))+1;
  std::vector<bool> fastFinder(sz,false);
  for(const mcIdType *work=begin;work!=end;work++)
    if(*work>=0 && *work<sz)
      fastFinder[*work]=true;
  const mcIdType *conn=_conn->begin();
  mcIdType nbNodesPerCell=getNumberOfNodesPerCell();
  for(mcIdType i=0;i<nbOfCells;i++,conn+=nbNodesPerCell)
    {
      int ref=0,nbOfHit=0;
      for(mcIdType j=0;j<nbNodesPerCell;j++)
        if(conn[j]>=0)
          {
            ref++;
            if(fastFinder[conn[j]])
              nbOfHit++;
          }
      if((ref==nbOfHit && fullyIn) || (nbOfHit!=0 && !fullyIn))
        cellIdsKept->pushBackSilent(i);
    }
  cellIdsKeptArr=cellIdsKept.retn();
}

MEDCouplingMesh *MEDCoupling1SGTUMesh::mergeMyselfWith(const MEDCouplingMesh *other) const
{
  if(other->getType()!=SINGLE_STATIC_GEO_TYPE_UNSTRUCTURED)
    throw INTERP_KERNEL::Exception("Merge of umesh only available with umesh single static geo type each other !");
  const MEDCoupling1SGTUMesh *otherC=static_cast<const MEDCoupling1SGTUMesh *>(other);
  return Merge1SGTUMeshes(this,otherC);
}

MEDCouplingUMesh *MEDCoupling1SGTUMesh::buildUnstructured() const
{
  MCAuto<MEDCouplingUMesh> ret=MEDCouplingUMesh::New(getName(),getMeshDimension());
  ret->setCoords(getCoords());
  const mcIdType *nodalConn=_conn->begin();
  mcIdType nbCells=getNumberOfCells();
  mcIdType nbNodesPerCell=getNumberOfNodesPerCell();
  mcIdType geoType=ToIdType(getCellModelEnum());
  MCAuto<DataArrayIdType> c=DataArrayIdType::New(); c->alloc(nbCells*(nbNodesPerCell+1),1);
  mcIdType *cPtr=c->getPointer();
  for(mcIdType i=0;i<nbCells;i++,nodalConn+=nbNodesPerCell)
    {
      *cPtr++=geoType;
      cPtr=std::copy(nodalConn,nodalConn+nbNodesPerCell,cPtr);
    }
  MCAuto<DataArrayIdType> cI=DataArrayIdType::Range(0,(nbCells+1)*(nbNodesPerCell+1),nbNodesPerCell+1);
  ret->setConnectivity(c,cI,true);
  try
  { ret->copyTinyInfoFrom(this); }
  catch(INTERP_KERNEL::Exception&) { }
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::simplexize(int policy)
{
  switch(policy)
  {
    case 0:
      return simplexizePol0();
    case 1:
      return simplexizePol1();
    case INTERP_KERNEL::PLANAR_FACE_5:
        return simplexizePlanarFace5();
    case INTERP_KERNEL::PLANAR_FACE_6:
        return simplexizePlanarFace6();
    default:
      throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::simplexize : unrecognized policy ! Must be :\n  - 0 or 1 (only available for meshdim=2) \n  - PLANAR_FACE_5, PLANAR_FACE_6  (only for meshdim=3)");
  }
}

/// @cond INTERNAL

struct MEDCouplingAccVisit
{
  MEDCouplingAccVisit():_new_nb_of_nodes(0) { }
  mcIdType operator()(mcIdType val) { if(val!=-1) return _new_nb_of_nodes++; else return -1; }
  mcIdType _new_nb_of_nodes;
};

/// @endcond

/*!
 * This method returns all node ids used in \b this. The data array returned has to be dealt by the caller.
 * The returned node ids are sortes ascendingly. This method is closed to MEDCoupling1SGTUMesh::getNodeIdsInUse except
 * the format of returned DataArrayIdType instance.
 *
 * \return a newly allocated DataArrayIdType sorted ascendingly of fetched node ids.
 * \sa MEDCoupling1SGTUMesh::getNodeIdsInUse, areAllNodesFetched
 */
DataArrayIdType *MEDCoupling1SGTUMesh::computeFetchedNodeIds() const
{
  checkConsistencyOfConnectivity();
  mcIdType nbNodes(getNumberOfNodes());
  std::vector<bool> fetchedNodes(nbNodes,false);
  computeNodeIdsAlg(fetchedNodes);
  mcIdType sz(ToIdType(std::count(fetchedNodes.begin(),fetchedNodes.end(),true)));
  MCAuto<DataArrayIdType> ret(DataArrayIdType::New()); ret->alloc(sz,1);
  mcIdType *retPtr(ret->getPointer());
  for(mcIdType i=0;i<nbNodes;i++)
    if(fetchedNodes[i])
      *retPtr++=i;
  return ret.retn();
}

/*!
 * Finds nodes not used in any cell and returns an array giving a new id to every node
 * by excluding the unused nodes, for which the array holds -1. The result array is
 * a mapping in "Old to New" mode. 
 *  \param [out] nbrOfNodesInUse - number of node ids present in the nodal connectivity.
 *  \return DataArrayIdType * - a new instance of DataArrayIdType. Its length is \a
 *          this->getNumberOfNodes(). It holds for each node of \a this mesh either -1
 *          if the node is unused or a new id else. The caller is to delete this
 *          array using decrRef() as it is no more needed.  
 *  \throw If the coordinates array is not set.
 *  \throw If the nodal connectivity of cells is not defined.
 *  \throw If the nodal connectivity includes an invalid id.
 *  \sa MEDCoupling1SGTUMesh::computeFetchedNodeIds, areAllNodesFetched
 */
DataArrayIdType *MEDCoupling1SGTUMesh::getNodeIdsInUse(mcIdType& nbrOfNodesInUse) const
{
  nbrOfNodesInUse=-1;
  mcIdType nbOfNodes=getNumberOfNodes();
  mcIdType nbOfCells=getNumberOfCells();
  MCAuto<DataArrayIdType> ret(DataArrayIdType::New());
  ret->alloc(nbOfNodes,1);
  mcIdType *traducer=ret->getPointer();
  std::fill(traducer,traducer+nbOfNodes,-1);
  const mcIdType *conn=_conn->begin();
  mcIdType nbNodesPerCell=getNumberOfNodesPerCell();
  for(mcIdType i=0;i<nbOfCells;i++)
    for(int j=0;j<nbNodesPerCell;j++,conn++)
      if(*conn>=0 && *conn<nbOfNodes)
        traducer[*conn]=1;
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::getNodeIdsInUse : In cell #" << i  << " presence of node id " <<  conn[j] << " not in [0," << nbOfNodes << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
  nbrOfNodesInUse=ToIdType(std::count(traducer,traducer+nbOfNodes,1));
  std::transform(traducer,traducer+nbOfNodes,traducer,MEDCouplingAccVisit());
  return ret.retn();
}

/*!
 * This method renumbers only nodal connectivity in \a this. The renumbering is only an offset applied. So this method is a specialization of
 * \a renumberNodesInConn. \b WARNING, this method does not check that the resulting node ids in the nodal connectivity is in a valid range !
 *
 * \param [in] offset - specifies the offset to be applied on each element of connectivity.
 *
 * \sa renumberNodesInConn
 */
void MEDCoupling1SGTUMesh::renumberNodesWithOffsetInConn(mcIdType offset)
{
  getNumberOfCells();//only to check that all is well defined.
  _conn->applyLin(1,offset);
  updateTime();
}

/*!
 *  Same than renumberNodesInConn(const mcIdType *) except that here the format of old-to-new traducer is using map instead
 *  of array. This method is dedicated for renumbering from a big set of nodes the a tiny set of nodes which is the case during extraction
 *  of a big mesh.
 */
void MEDCoupling1SGTUMesh::renumberNodesInConn(const INTERP_KERNEL::HashMap<mcIdType,mcIdType>& newNodeNumbersO2N)
{
  this->renumberNodesInConnT< INTERP_KERNEL::HashMap<mcIdType,mcIdType> >(newNodeNumbersO2N);
}

/*!
 *  Same than renumberNodesInConn(const mcIdType *) except that here the format of old-to-new traducer is using map instead
 *  of array. This method is dedicated for renumbering from a big set of nodes the a tiny set of nodes which is the case during extraction
 *  of a big mesh.
 */
void MEDCoupling1SGTUMesh::renumberNodesInConn(const std::map<mcIdType,mcIdType>& newNodeNumbersO2N)
{
  this->renumberNodesInConnT< std::map<mcIdType,mcIdType> >(newNodeNumbersO2N);
}

/*!
 * Changes ids of nodes within the nodal connectivity arrays according to a permutation
 * array in "Old to New" mode. The node coordinates array is \b not changed by this method.
 * This method is a generalization of shiftNodeNumbersInConn().
 *  \warning This method performs no check of validity of new ids. **Use it with care !**
 *  \param [in] newNodeNumbersO2N - a permutation array, of length \a
 *         this->getNumberOfNodes(), in "Old to New" mode. 
 *         See \ref numbering for more info on renumbering modes.
 *  \throw If the nodal connectivity of cells is not defined.
 */
void MEDCoupling1SGTUMesh::renumberNodesInConn(const mcIdType *newNodeNumbersO2N)
{
  getNumberOfCells();//only to check that all is well defined.
  _conn->transformWithIndArr(newNodeNumbersO2N,newNodeNumbersO2N+getNumberOfNodes());
  updateTime();
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::Merge1SGTUMeshes(const MEDCoupling1SGTUMesh *mesh1, const MEDCoupling1SGTUMesh *mesh2)
{
  std::vector<const MEDCoupling1SGTUMesh *> tmp(2);
  tmp[0]=const_cast<MEDCoupling1SGTUMesh *>(mesh1); tmp[1]=const_cast<MEDCoupling1SGTUMesh *>(mesh2);
  return Merge1SGTUMeshes(tmp);
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::Merge1SGTUMeshes(std::vector<const MEDCoupling1SGTUMesh *>& a)
{
  std::size_t sz=a.size();
  if(sz==0)
    return Merge1SGTUMeshesLL(a);
  for(std::size_t ii=0;ii<sz;ii++)
    if(!a[ii])
      {
        std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::Merge1SGTUMeshes : item #" << ii << " in input array of size "<< sz << " is empty !";
        throw INTERP_KERNEL::Exception(oss.str().c_str());
      }
  const INTERP_KERNEL::CellModel *cm=&(a[0]->getCellModel());
  for(std::size_t ii=0;ii<sz;ii++)
    if(&(a[ii]->getCellModel())!=cm)
      throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshes : all items must have the same geo type !");
  std::vector< MCAuto<MEDCoupling1SGTUMesh> > bb(sz);
  std::vector< const MEDCoupling1SGTUMesh * > aa(sz);
  std::size_t spaceDimUndef=-3, spaceDim=spaceDimUndef;
  for(std::size_t i=0;i<sz && spaceDim==spaceDimUndef;i++)
    {
      const MEDCoupling1SGTUMesh *cur=a[i];
      const DataArrayDouble *coo=cur->getCoords();
      if(coo)
        spaceDim=coo->getNumberOfComponents();
    }
  if(spaceDim==spaceDimUndef)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshes : no spaceDim specified ! unable to perform merge !");
  for(std::size_t i=0;i<sz;i++)
    {
      bb[i]=a[i]->buildSetInstanceFromThis(spaceDim);
      aa[i]=bb[i];
    }
  return Merge1SGTUMeshesLL(aa);
}

/*!
 * \throw If presence of a null instance in the input vector \a a.
 * \throw If a is empty
 */
MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::Merge1SGTUMeshesOnSameCoords(std::vector<const MEDCoupling1SGTUMesh *>& a)
{
  if(a.empty())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshesOnSameCoords : input array must be NON EMPTY !");
  std::vector<const MEDCoupling1SGTUMesh *>::const_iterator it=a.begin();
  if(!(*it))
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshesOnSameCoords : null instance in the first element of input vector !");
  std::vector<const DataArrayIdType *> ncs(a.size());
  (*it)->getNumberOfCells();//to check that all is OK
  const DataArrayDouble *coords=(*it)->getCoords();
  const INTERP_KERNEL::CellModel *cm=&((*it)->getCellModel());
  ncs[0]=(*it)->getNodalConnectivity();
  it++;
  for(int i=1;it!=a.end();i++,it++)
    {
      if(!(*it))
        throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshesOnSameCoords : presence of a null instance in the input vector !");
      if(cm!=&((*it)->getCellModel()))
        throw INTERP_KERNEL::Exception("Geometric types mismatches, Merge1SGTUMeshes impossible !");
      (*it)->getNumberOfCells();//to check that all is OK
      ncs[i]=(*it)->getNodalConnectivity();
      if(coords!=(*it)->getCoords())
        throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshesOnSameCoords : not lying on same coords !");
    }
  MCAuto<MEDCoupling1SGTUMesh> ret(new MEDCoupling1SGTUMesh("merge",*cm));
  ret->setCoords(coords);
  ret->_conn=DataArrayIdType::Aggregate(ncs);
  return ret.retn();
}

/*!
 * Assume that all instances in \a a are non null. If null it leads to a crash. That's why this method is assigned to be low level (LL)
 */
MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::Merge1SGTUMeshesLL(std::vector<const MEDCoupling1SGTUMesh *>& a)
{
  if(a.empty())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::Merge1SGTUMeshes : input array must be NON EMPTY !");
  std::vector<const MEDCoupling1SGTUMesh *>::const_iterator it=a.begin();
  mcIdType nbOfCells=(*it)->getNumberOfCells();
  const INTERP_KERNEL::CellModel *cm=&((*it)->getCellModel());
  mcIdType nbNodesPerCell=(*it)->getNumberOfNodesPerCell();
  it++;
  for(;it!=a.end();it++)
    {
      if(cm!=&((*it)->getCellModel()))
        throw INTERP_KERNEL::Exception("Geometric types mismatches, Merge1SGTUMeshes impossible !");
      nbOfCells+=(*it)->getNumberOfCells();
    }
  std::vector<const MEDCouplingPointSet *> aps(a.size());
  std::copy(a.begin(),a.end(),aps.begin());
  MCAuto<DataArrayDouble> pts=MergeNodesArray(aps);
  MCAuto<MEDCoupling1SGTUMesh> ret(new MEDCoupling1SGTUMesh("merge",*cm));
  ret->setCoords(pts);
  MCAuto<DataArrayIdType> c=DataArrayIdType::New();
  c->alloc(nbOfCells*nbNodesPerCell,1);
  mcIdType *cPtr=c->getPointer();
  mcIdType offset=0;
  for(it=a.begin();it!=a.end();it++)
    {
      mcIdType curConnLgth=(*it)->getNodalConnectivityLength();
      const mcIdType *curC=(*it)->_conn->begin();
      cPtr=std::transform(curC,curC+curConnLgth,cPtr,std::bind(std::plus<mcIdType>(),std::placeholders::_1,offset));
      offset+=(*it)->getNumberOfNodes();
    }
  //
  ret->setNodalConnectivity(c);
  return ret.retn();
}

MEDCouplingPointSet *MEDCoupling1SGTUMesh::buildPartOfMySelfKeepCoords(const mcIdType *begin, const mcIdType *end) const
{
  mcIdType ncell=getNumberOfCells();
  MCAuto<MEDCoupling1SGTUMesh> ret(new MEDCoupling1SGTUMesh(getName(),*_cm));
  ret->setCoords(_coords);
  std::size_t nbOfElemsRet=std::distance(begin,end);
  const mcIdType *inConn=_conn->getConstPointer();
  mcIdType sz=getNumberOfNodesPerCell();
  MCAuto<DataArrayIdType> connRet=DataArrayIdType::New(); connRet->alloc(nbOfElemsRet*sz,1);
  mcIdType *connPtr=connRet->getPointer();
  for(const mcIdType *work=begin;work!=end;work++,connPtr+=sz)
    {
      if(*work>=0 && *work<ncell)
        std::copy(inConn+(work[0])*sz,inConn+(work[0]+1)*sz,connPtr);
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::buildPartOfMySelfKeepCoords : On pos #" << std::distance(begin,work) << " input cell id =" << *work << " should be in [0," << ncell << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  ret->_conn=connRet;
  ret->copyTinyInfoFrom(this);
  return ret.retn();
}

MEDCouplingPointSet *MEDCoupling1SGTUMesh::buildPartOfMySelfKeepCoordsSlice(mcIdType start, mcIdType end, mcIdType step) const
{
  mcIdType ncell=getNumberOfCells();
  mcIdType nbOfElemsRet=DataArray::GetNumberOfItemGivenBESRelative(start,end,step,"MEDCoupling1SGTUMesh::buildPartOfMySelfKeepCoordsSlice : ");
  MCAuto<MEDCoupling1SGTUMesh> ret(new MEDCoupling1SGTUMesh(getName(),*_cm));
  ret->setCoords(_coords);
  const mcIdType *inConn=_conn->getConstPointer();
  mcIdType sz=getNumberOfNodesPerCell();
  MCAuto<DataArrayIdType> connRet=DataArrayIdType::New(); connRet->alloc(nbOfElemsRet*sz,1);
  mcIdType *connPtr=connRet->getPointer();
  mcIdType curId=start;
  for(mcIdType i=0;i<nbOfElemsRet;i++,connPtr+=sz,curId+=step)
    {
      if(curId>=0 && curId<ncell)
        std::copy(inConn+curId*sz,inConn+(curId+1)*sz,connPtr);
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::buildPartOfMySelfKeepCoordsSlice : On pos #" << i << " input cell id =" << curId  << " should be in [0," << ncell << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  ret->_conn=connRet;
  ret->copyTinyInfoFrom(this);
  return ret.retn();
}

void MEDCoupling1SGTUMesh::computeNodeIdsAlg(std::vector<bool>& nodeIdsInUse) const
{
  mcIdType sz(ToIdType(nodeIdsInUse.size()));
  for(const mcIdType *conn=_conn->begin();conn!=_conn->end();conn++)
    {
      if(*conn>=0 && *conn<sz)
       nodeIdsInUse[*conn]=true;
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::computeFetchedNodeIds : At pos #" << std::distance(_conn->begin(),conn) << " value is " << *conn << " must be in [0," << sz << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
}

MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::buildSetInstanceFromThis(std::size_t spaceDim) const
{
  MCAuto<MEDCoupling1SGTUMesh> ret(new MEDCoupling1SGTUMesh(getName(),*_cm));
  MCAuto<DataArrayIdType> tmp1;
  const DataArrayIdType *nodalConn(_conn);
  if(!nodalConn)
    {
      tmp1=DataArrayIdType::New(); tmp1->alloc(0,1);
    }
  else
    tmp1=_conn;
  ret->_conn=tmp1;
  if(!_coords)
    {
      MCAuto<DataArrayDouble> coords=DataArrayDouble::New(); coords->alloc(0,spaceDim);
      ret->setCoords(coords);
    }
  else
    ret->setCoords(_coords);
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::simplexizePol0()
{
  mcIdType nbOfCells=getNumberOfCells();
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_QUAD4)
    return DataArrayIdType::Range(0,nbOfCells,1);
  MCAuto<DataArrayIdType> newConn=DataArrayIdType::New(); newConn->alloc(2*3*nbOfCells,1);
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New(); ret->alloc(2*nbOfCells,1);
  const mcIdType *c(_conn->begin());
  mcIdType *retPtr(ret->getPointer()),*newConnPtr(newConn->getPointer());
  for(mcIdType i=0;i<nbOfCells;i++,c+=4,newConnPtr+=6,retPtr+=2)
    {
      newConnPtr[0]=c[0]; newConnPtr[1]=c[1]; newConnPtr[2]=c[2];
      newConnPtr[3]=c[0]; newConnPtr[4]=c[2]; newConnPtr[5]=c[3];
      retPtr[0]=i; retPtr[1]=i;
    }
  _conn=newConn;
  _cm=&INTERP_KERNEL::CellModel::GetCellModel(INTERP_KERNEL::NORM_TRI3);
  updateTime();
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::simplexizePol1()
{
  mcIdType nbOfCells=getNumberOfCells();
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_QUAD4)
    return DataArrayIdType::Range(0,nbOfCells,1);
  MCAuto<DataArrayIdType> newConn=DataArrayIdType::New(); newConn->alloc(2*3*nbOfCells,1);
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New(); ret->alloc(2*nbOfCells,1);
  const mcIdType *c(_conn->begin());
  mcIdType *retPtr(ret->getPointer()),*newConnPtr(newConn->getPointer());
  for(mcIdType i=0;i<nbOfCells;i++,c+=4,newConnPtr+=6,retPtr+=2)
    {
      newConnPtr[0]=c[0]; newConnPtr[1]=c[1]; newConnPtr[2]=c[3];
      newConnPtr[3]=c[1]; newConnPtr[4]=c[2]; newConnPtr[5]=c[3];
      retPtr[0]=i; retPtr[1]=i;
    }
  _conn=newConn;
  _cm=&INTERP_KERNEL::CellModel::GetCellModel(INTERP_KERNEL::NORM_TRI3);
  updateTime();
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::simplexizePlanarFace5()
{
  mcIdType nbOfCells=getNumberOfCells();
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_HEXA8)
    return DataArrayIdType::Range(0,nbOfCells,1);
  MCAuto<DataArrayIdType> newConn=DataArrayIdType::New(); newConn->alloc(5*4*nbOfCells,1);
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New(); ret->alloc(5*nbOfCells,1);
  const mcIdType *c(_conn->begin());
  mcIdType *retPtr(ret->getPointer()),*newConnPtr(newConn->getPointer());
  for(mcIdType i=0;i<nbOfCells;i++,c+=8,newConnPtr+=20,retPtr+=5)
    {
      for(int j=0;j<20;j++)
        newConnPtr[j]=c[INTERP_KERNEL::SPLIT_NODES_5_WO[j]];
      retPtr[0]=i; retPtr[1]=i; retPtr[2]=i; retPtr[3]=i; retPtr[4]=i;
    }
  _conn=newConn;
  _cm=&INTERP_KERNEL::CellModel::GetCellModel(INTERP_KERNEL::NORM_TETRA4);
  updateTime();
  return ret.retn();
}

DataArrayIdType *MEDCoupling1SGTUMesh::simplexizePlanarFace6()
{
  mcIdType nbOfCells=getNumberOfCells();
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_HEXA8)
    return DataArrayIdType::Range(0,nbOfCells,1);
  MCAuto<DataArrayIdType> newConn=DataArrayIdType::New(); newConn->alloc(6*4*nbOfCells,1);
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New(); ret->alloc(6*nbOfCells,1);
  const mcIdType *c(_conn->begin());
  mcIdType *retPtr(ret->getPointer()),*newConnPtr(newConn->getPointer());
  for(mcIdType i=0;i<nbOfCells;i++,c+=8,newConnPtr+=24,retPtr+=6)
    {
      for(int j=0;j<24;j++)
        newConnPtr[j]=c[INTERP_KERNEL::SPLIT_NODES_6_WO[j]];
      retPtr[0]=i; retPtr[1]=i; retPtr[2]=i; retPtr[3]=i; retPtr[4]=i; retPtr[5]=i;
    }
  _conn=newConn;
  _cm=&INTERP_KERNEL::CellModel::GetCellModel(INTERP_KERNEL::NORM_TETRA4);
  updateTime();
  return ret.retn();
}

void MEDCoupling1SGTUMesh::reprQuickOverview(std::ostream& stream) const
{
  stream << "MEDCoupling1SGTUMesh C++ instance at " << this << ". Type=";
  if(!_cm)
  {
    stream << "Not set";
    return ;
  }
  stream << _cm->getRepr() << ". Name : \"" << getName() << "\".";
  stream << " Mesh dimension : " << getMeshDimension() << ".";
  if(!_coords)
    { stream << " No coordinates set !"; return ; }
  if(!_coords->isAllocated())
    { stream << " Coordinates set but not allocated !"; return ; }
  stream << " Space dimension : " << _coords->getNumberOfComponents() << "." << std::endl;
  stream << "Number of nodes : " << _coords->getNumberOfTuples() << ".";
  if(!(const DataArrayIdType *)_conn)
    { stream << std::endl << "Nodal connectivity NOT set !"; return ; }
  if(_conn->isAllocated())
    {
      if(_conn->getNumberOfComponents()==1)
        stream << std::endl << "Number of cells : " << getNumberOfCells() << ".";
    }
}

void MEDCoupling1SGTUMesh::checkFullyDefined() const
{
  if(!((const DataArrayIdType *)_conn) || !((const DataArrayDouble *)_coords))
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkFullyDefined : part of this is not fully defined.");
}

/*!
 * First step of unserialization process.
 */
bool MEDCoupling1SGTUMesh::isEmptyMesh(const std::vector<mcIdType>& tinyInfo) const
{
  throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::isEmptyMesh : not implemented yet !");
}

void MEDCoupling1SGTUMesh::getTinySerializationInformation(std::vector<double>& tinyInfoD, std::vector<mcIdType>& tinyInfo, std::vector<std::string>& littleStrings) const
{
  int it,order;
  double time=getTime(it,order);
  tinyInfo.clear(); tinyInfoD.clear(); littleStrings.clear();
  //
  littleStrings.push_back(getName());
  littleStrings.push_back(getDescription());
  littleStrings.push_back(getTimeUnit());
  //
  std::vector<std::string> littleStrings2,littleStrings3;
  if((const DataArrayDouble *)_coords)
    _coords->getTinySerializationStrInformation(littleStrings2);
  if((const DataArrayIdType *)_conn)
    _conn->getTinySerializationStrInformation(littleStrings3);
  mcIdType sz0(ToIdType(littleStrings2.size())),sz1(ToIdType(littleStrings3.size()));
  littleStrings.insert(littleStrings.end(),littleStrings2.begin(),littleStrings2.end());
  littleStrings.insert(littleStrings.end(),littleStrings3.begin(),littleStrings3.end());
  //
  tinyInfo.push_back(getCellModelEnum());
  tinyInfo.push_back(it);
  tinyInfo.push_back(order);
  std::vector<mcIdType> tinyInfo2,tinyInfo3;
  if((const DataArrayDouble *)_coords)
    _coords->getTinySerializationIntInformation(tinyInfo2);
  if((const DataArrayIdType *)_conn)
    _conn->getTinySerializationIntInformation(tinyInfo3);
  mcIdType sz2(ToIdType(tinyInfo2.size())),sz3(ToIdType(tinyInfo3.size()));
  tinyInfo.push_back(sz0); tinyInfo.push_back(sz1); tinyInfo.push_back(sz2); tinyInfo.push_back(sz3);
  tinyInfo.insert(tinyInfo.end(),tinyInfo2.begin(),tinyInfo2.end());
  tinyInfo.insert(tinyInfo.end(),tinyInfo3.begin(),tinyInfo3.end());
  //
  tinyInfoD.push_back(time);
}

void MEDCoupling1SGTUMesh::resizeForUnserialization(const std::vector<mcIdType>& tinyInfo, DataArrayIdType *a1, DataArrayDouble *a2, std::vector<std::string>& littleStrings) const
{
  std::vector<mcIdType> tinyInfo2(tinyInfo.begin()+7,tinyInfo.begin()+7+tinyInfo[5]);
  std::vector<mcIdType> tinyInfo1(tinyInfo.begin()+7+tinyInfo[5],tinyInfo.begin()+7+tinyInfo[5]+tinyInfo[6]);
  a1->resizeForUnserialization(tinyInfo1);
  a2->resizeForUnserialization(tinyInfo2);
}

void MEDCoupling1SGTUMesh::serialize(DataArrayIdType *&a1, DataArrayDouble *&a2) const
{
  mcIdType sz(0);
  if((const DataArrayIdType *)_conn)
    if(_conn->isAllocated())
      sz=_conn->getNbOfElems();
  a1=DataArrayIdType::New();
  a1->alloc(sz,1);
  if(sz!=0 && (const DataArrayIdType *)_conn)
    std::copy(_conn->begin(),_conn->end(),a1->getPointer());
  sz=0;
  if((const DataArrayDouble *)_coords)
    if(_coords->isAllocated())
      sz=_coords->getNbOfElems();
  a2=DataArrayDouble::New();
  a2->alloc(sz,1);
  if(sz!=0 && (const DataArrayDouble *)_coords)
    std::copy(_coords->begin(),_coords->end(),a2->getPointer());
}

void MEDCoupling1SGTUMesh::unserialization(const std::vector<double>& tinyInfoD, const std::vector<mcIdType>& tinyInfo, const DataArrayIdType *a1, DataArrayDouble *a2,
                                           const std::vector<std::string>& littleStrings)
{
  INTERP_KERNEL::NormalizedCellType gt((INTERP_KERNEL::NormalizedCellType)tinyInfo[0]);
  _cm=&INTERP_KERNEL::CellModel::GetCellModel(gt);
  setName(littleStrings[0]);
  setDescription(littleStrings[1]);
  setTimeUnit(littleStrings[2]);
  setTime(tinyInfoD[0],FromIdType<int>(tinyInfo[1]),FromIdType<int>(tinyInfo[2]));
  mcIdType sz0(tinyInfo[3]),sz1(tinyInfo[4]),sz2(tinyInfo[5]),sz3(tinyInfo[6]);
  //
  _coords=DataArrayDouble::New();
  std::vector<mcIdType> tinyInfo2(tinyInfo.begin()+7,tinyInfo.begin()+7+sz2);
  _coords->resizeForUnserialization(tinyInfo2);
  std::copy(a2->begin(),a2->end(),_coords->getPointer());
  _conn=DataArrayIdType::New();
  std::vector<mcIdType> tinyInfo3(tinyInfo.begin()+7+sz2,tinyInfo.begin()+7+sz2+sz3);
  _conn->resizeForUnserialization(tinyInfo3);
  std::copy(a1->begin(),a1->end(),_conn->getPointer());
  std::vector<std::string> littleStrings2(littleStrings.begin()+3,littleStrings.begin()+3+sz0);
  _coords->finishUnserialization(tinyInfo2,littleStrings2);
  std::vector<std::string> littleStrings3(littleStrings.begin()+3+sz0,littleStrings.begin()+3+sz0+sz1);
  _conn->finishUnserialization(tinyInfo3,littleStrings3);
}

/*!
 * Checks if \a this and \a other meshes are geometrically equivalent with high
 * probability, else an exception is thrown. The meshes are considered equivalent if
 * (1) meshes contain the same number of nodes and the same number of elements of the
 * same types (2) three cells of the two meshes (first, last and middle) are based
 * on coincident nodes (with a specified precision).
 *  \param [in] other - the mesh to compare with.
 *  \param [in] prec - the precision used to compare nodes of the two meshes.
 *  \throw If the two meshes do not match.
 */
void MEDCoupling1SGTUMesh::checkFastEquivalWith(const MEDCouplingMesh *other, double prec) const
{
  MEDCouplingPointSet::checkFastEquivalWith(other,prec);
  const MEDCoupling1SGTUMesh *otherC=dynamic_cast<const MEDCoupling1SGTUMesh *>(other);
  if(!otherC)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkFastEquivalWith : Two meshes are not unstructured with single static geometric type !");
  const DataArrayIdType *c1(_conn),*c2(otherC->_conn);
  if(c1==c2)
    return;
  if(!c1 || !c2)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkFastEquivalWith : presence of nodal connectivity only in one of the 2 meshes !");
  if((c1->isAllocated() && !c2->isAllocated()) || (!c1->isAllocated() && c2->isAllocated()))
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkFastEquivalWith : in nodal connectivity, only one is allocated !");
  if(c1->getNumberOfComponents()!=1 || c1->getNumberOfComponents()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkFastEquivalWith : in nodal connectivity, must have 1 and only 1 component !");
  if(c1->getHashCode()!=c2->getHashCode())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::checkFastEquivalWith : nodal connectivity differs");
}

MEDCouplingPointSet *MEDCoupling1SGTUMesh::mergeMyselfWithOnSameCoords(const MEDCouplingPointSet *other) const
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::mergeMyselfWithOnSameCoords : input other is null !");
  const MEDCoupling1SGTUMesh *otherC=dynamic_cast<const MEDCoupling1SGTUMesh *>(other);
  if(!otherC)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::mergeMyselfWithOnSameCoords : the input other mesh is not of type single statuc geo type unstructured !");
  std::vector<const MEDCoupling1SGTUMesh *> ms(2);
  ms[0]=this;
  ms[1]=otherC;
  return Merge1SGTUMeshesOnSameCoords(ms);
}

void MEDCoupling1SGTUMesh::getReverseNodalConnectivity(DataArrayIdType *revNodal, DataArrayIdType *revNodalIndx) const
{
  checkFullyDefined();
  mcIdType nbOfNodes=getNumberOfNodes();
  mcIdType *revNodalIndxPtr=(mcIdType *)malloc((nbOfNodes+1)*sizeof(mcIdType));
  revNodalIndx->useArray(revNodalIndxPtr,true,DeallocType::C_DEALLOC,nbOfNodes+1,1);
  std::fill(revNodalIndxPtr,revNodalIndxPtr+nbOfNodes+1,0);
  const mcIdType *conn=_conn->begin();
  mcIdType nbOfCells=getNumberOfCells();
  mcIdType nbOfEltsInRevNodal=0;
  mcIdType nbOfNodesPerCell=getNumberOfNodesPerCell();
  for(mcIdType eltId=0;eltId<nbOfCells;eltId++)
    {
      for(int j=0;j<nbOfNodesPerCell;j++,conn++)
        {
          if(conn[0]>=0 && conn[0]<nbOfNodes)
            {
              nbOfEltsInRevNodal++;
              revNodalIndxPtr[conn[0]+1]++;
            }
          else
            {
              std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::getReverseNodalConnectivity : At cell #" << eltId << " presence of nodeId #" << conn[0] << " should be in [0," << nbOfNodes << ") !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
    }
  std::transform(revNodalIndxPtr+1,revNodalIndxPtr+nbOfNodes+1,revNodalIndxPtr,revNodalIndxPtr+1,std::plus<mcIdType>());
  conn=_conn->begin();
  mcIdType *revNodalPtr=(mcIdType *)malloc(nbOfEltsInRevNodal*sizeof(mcIdType));
  revNodal->useArray(revNodalPtr,true,DeallocType::C_DEALLOC,nbOfEltsInRevNodal,1);
  std::fill(revNodalPtr,revNodalPtr+nbOfEltsInRevNodal,-1);
  for(mcIdType eltId=0;eltId<nbOfCells;eltId++)
    {
      for(int j=0;j<nbOfNodesPerCell;j++,conn++)
        {
          *std::find_if(revNodalPtr+revNodalIndxPtr[*conn],revNodalPtr+revNodalIndxPtr[*conn+1],std::bind(std::equal_to<mcIdType>(),std::placeholders::_1,-1))=eltId;
        }
    }
}

/*!
 * Use \a nodalConn array as nodal connectivity of \a this. The input \a nodalConn pointer can be null.
 */
void MEDCoupling1SGTUMesh::setNodalConnectivity(DataArrayIdType *nodalConn)
{
  if(nodalConn)
    nodalConn->incrRef();
  _conn=nodalConn;
  declareAsNew();
}

/*!
 * \return DataArrayIdType * - the internal reference to the nodal connectivity. The caller is not responsible to deallocate it.
 */
DataArrayIdType *MEDCoupling1SGTUMesh::getNodalConnectivity() const
{
  const DataArrayIdType *ret(_conn);
  return const_cast<DataArrayIdType *>(ret);
}

/*!
 * Allocates memory to store an estimation of the given number of cells. Closer is the estimation to the number of cells effectively inserted,
 * less will be the needs to realloc. If the number of cells to be inserted is not known simply put 0 to this parameter.
 * If a nodal connectivity previously existed before the call of this method, it will be reset.
 *
 *  \param [in] nbOfCells - estimation of the number of cell \a this mesh will contain.
 */
void MEDCoupling1SGTUMesh::allocateCells(mcIdType nbOfCells)
{
  if(nbOfCells<0)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::allocateCells : the input number of cells should be >= 0 !");
  _conn=DataArrayIdType::New();
  _conn->reserve(getNumberOfNodesPerCell()*nbOfCells);
  declareAsNew();
}

/*!
 * Appends at the end of \a this a cell having nodal connectivity array defined in [ \a nodalConnOfCellBg, \a nodalConnOfCellEnd ).
 *
 * \param [in] nodalConnOfCellBg - the begin (included) of nodal connectivity of the cell to add.
 * \param [in] nodalConnOfCellEnd - the end (excluded) of nodal connectivity of the cell to add.
 * \throw If the length of the input nodal connectivity array of the cell to add is not equal to number of nodes per cell relative to the unique geometric type
 *        attached to \a this.
 * \throw If the nodal connectivity array in \a this is null (call MEDCoupling1SGTUMesh::allocateCells before).
 */
void MEDCoupling1SGTUMesh::insertNextCell(const mcIdType *nodalConnOfCellBg, const mcIdType *nodalConnOfCellEnd)
{
  mcIdType sz=ToIdType(std::distance(nodalConnOfCellBg,nodalConnOfCellEnd));
  mcIdType ref=getNumberOfNodesPerCell();
  if(sz==ref)
    {
      DataArrayIdType *c(_conn);
      if(c)
        c->pushBackValsSilent(nodalConnOfCellBg,nodalConnOfCellEnd);
      else
        throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::insertNextCell : nodal connectivity array is null ! Call MEDCoupling1SGTUMesh::allocateCells before !");
    }
  else
    {
      std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::insertNextCell : input nodal size (" << sz << ") does not match number of nodes per cell of this (";
      oss << ref << ") !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
}

/*!
 * This method builds the dual mesh of \a this and returns it.
 * 
 * \return MEDCoupling1SGTUMesh * - newly object created to be managed by the caller.
 * \throw If \a this is not a mesh containing only simplex cells.
 * \throw If \a this is not correctly allocated (coordinates and connectivities have to be correctly set !).
 * \throw If at least one node in \a this is orphan (without any simplex cell lying on it !)
 */
MEDCoupling1GTUMesh *MEDCoupling1SGTUMesh::computeDualMesh() const
{
  const INTERP_KERNEL::CellModel& cm(getCellModel());
  if(!cm.isSimplex())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::computeDualMesh : this mesh is not a simplex mesh ! Please invoke simplexize of tetrahedrize on this before calling this method !");
  switch(getMeshDimension())
  {
    case 3:
      return computeDualMesh3D();
    case 2:
      return computeDualMesh2D();
    default:
      throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::computeDualMesh : meshdimension must be in [2,3] !");
  }
}

/*!
 * This method explode each NORM_HEXA8 cells in \a this into 6 NORM_QUAD4 cells and put the result into the MEDCoupling1SGTUMesh returned instance.
 * 
 * \return MEDCoupling1SGTUMesh * - a newly allocated instances (to be managed by the caller) storing the result of the explosion.
 * \throw If \a this is not a mesh containing only NORM_HEXA8 cells.
 * \throw If \a this is not properly allocated.
 */
MEDCoupling1SGTUMesh *MEDCoupling1SGTUMesh::explodeEachHexa8To6Quad4() const
{
  const INTERP_KERNEL::CellModel& cm(getCellModel());
  if(cm.getEnum()!=INTERP_KERNEL::NORM_HEXA8)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::explodeEachHexa8To6Quad4 : this method can be applied only on HEXA8 mesh !");
  mcIdType nbHexa8=getNumberOfCells();
  const mcIdType *inConnPtr(getNodalConnectivity()->begin());
  MCAuto<MEDCoupling1SGTUMesh> ret(MEDCoupling1SGTUMesh::New(getName(),INTERP_KERNEL::NORM_QUAD4));
  MCAuto<DataArrayIdType> c(DataArrayIdType::New()); c->alloc(nbHexa8*6*4,1);
  mcIdType *cPtr(c->getPointer());
  for(mcIdType i=0;i<nbHexa8;i++,inConnPtr+=8)
    {
      for(int j=0;j<6;j++,cPtr+=4)
        cm.fillSonCellNodalConnectivity(j,inConnPtr,cPtr);
    }
  ret->setCoords(getCoords());
  ret->setNodalConnectivity(c);
  return ret.retn();
}

/*!
 * This method for each cell in \a this the triangle height for each edge in a newly allocated/created array instance.
 *
 * \return DataArrayDouble * - a newly allocated instance with this->getNumberOfCells() tuples and 3 components storing for each cell in \a this the corresponding  height.
 * \throw If \a this is not a mesh containing only NORM_TRI3 cells.
 * \throw If \a this is not properly allocated.
 * \throw If spaceDimension is not in 2 or 3.
 */
MCAuto<DataArrayDouble> MEDCoupling1SGTUMesh::computeTriangleHeight() const
{
  checkConsistencyLight();
  const INTERP_KERNEL::CellModel& cm(getCellModel());
  if(cm.getEnum()!=INTERP_KERNEL::NORM_TRI3)
    THROW_IK_EXCEPTION("MEDCoupling1SGTUMesh::computeTriangleHeight : this method can be applied only on TRI3 mesh !");
  MCAuto<DataArrayDouble> ret(DataArrayDouble::New());
  mcIdType nbTri3( getNumberOfCells() );
  const double *coordPtr( this->getCoords()->begin() );
  const mcIdType *inConnPtr(getNodalConnectivity()->begin());
  ret->alloc(nbTri3,3);
  double *retPtr( ret->getPointer() );
  switch( this->getSpaceDimension())
  {
    case 2:
    {
      constexpr unsigned SPACEDIM = 2;
      for(mcIdType iCell = 0 ; iCell < nbTri3 ; ++iCell)
      {
        INTERP_KERNEL::ComputeTriangleHeight<SPACEDIM>(coordPtr + SPACEDIM*inConnPtr[3*iCell+0], coordPtr + SPACEDIM*inConnPtr[3*iCell+1], coordPtr + SPACEDIM*inConnPtr[3*iCell+2],retPtr+3*iCell);
      }
      break;
    }
    case 3:
    {
      constexpr unsigned SPACEDIM = 3;
      for(mcIdType iCell = 0 ; iCell < nbTri3 ; ++iCell)
      {
        INTERP_KERNEL::ComputeTriangleHeight<SPACEDIM>(coordPtr + SPACEDIM*inConnPtr[3*iCell+0], coordPtr + SPACEDIM*inConnPtr[3*iCell+1], coordPtr + SPACEDIM*inConnPtr[3*iCell+2],retPtr+3*iCell);
      }
      break;
    }
    default:
      THROW_IK_EXCEPTION("MEDCoupling1SGTUMesh::computeTriangleHeight : only spacedim in [2,3] supported !");
  }
  return ret;
}

/*!
 * This method starts from an unstructured mesh that hides in reality a cartesian mesh.
 * If it is not the case, an exception will be thrown.
 * This method returns three objects : The cartesian mesh geometrically equivalent to \a this (within a precision of \a eps) and a permutation of cells
 * and a permutation of nodes.
 *
 * - this[cellPerm[i]]=ret[i]
 *
 * \param [out] cellPerm the permutation array of size \c this->getNumberOfCells()
 * \param [out] nodePerm the permutation array of size \c this->getNumberOfNodes()
 * \return MEDCouplingCMesh * - a newly allocated mesh that is the result of the structurization of \a this.
 */
MEDCouplingCMesh *MEDCoupling1SGTUMesh::structurizeMe(DataArrayIdType *& cellPerm, DataArrayIdType *& nodePerm, double eps) const
{
  checkConsistencyLight();
  int spaceDim(getSpaceDimension()),meshDim(getMeshDimension()); mcIdType nbNodes(getNumberOfNodes());
  if(MEDCouplingStructuredMesh::GetGeoTypeGivenMeshDimension(meshDim)!=getCellModelEnum())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::structurizeMe : the unique geo type in this is not compatible with the geometric type regarding mesh dimension !");
  MCAuto<MEDCouplingCMesh> cm(MEDCouplingCMesh::New());
  for(int i=0;i<spaceDim;i++)
    {
      std::vector<std::size_t> tmp(1,i);
      MCAuto<DataArrayDouble> elt(static_cast<DataArrayDouble*>(getCoords()->keepSelectedComponents(tmp)));
      elt=elt->getDifferentValues(eps);
      elt->sort(true);
      cm->setCoordsAt(i,elt);
    }
  if(nbNodes!=cm->getNumberOfNodes())
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::structurizeMe : considering the number of nodes after split per components in space this can't be a cartesian mesh ! Maybe your epsilon parameter is invalid ?");
  try
  { cm->copyTinyInfoFrom(this); }
  catch(INTERP_KERNEL::Exception&) { }
  MCAuto<MEDCouplingUMesh> um(cm->buildUnstructured()),self(buildUnstructured());
  self->checkGeoEquivalWith(um,12,eps,cellPerm,nodePerm);
  return cm.retn();
}

/// @cond INTERNAL

bool UpdateHexa8Cell(int validAxis, mcIdType neighId, const mcIdType *validConnQuad4NeighSide, mcIdType *allFacesNodalConn, mcIdType *myNeighbours)
{
  static const int TAB[48]={
    0,1,2,3,4,5,6,7,//0
    4,7,6,5,0,3,2,1,//1
    0,3,7,4,1,2,6,5,//2
    4,0,3,7,5,1,2,6,//3
    5,1,0,4,6,2,3,7,//4
    3,7,4,0,2,6,5,1 //5
  };
  static const int TAB2[6]={0,0,3,3,3,3};
  if(myNeighbours[validAxis]==neighId && allFacesNodalConn[4*validAxis+0]==validConnQuad4NeighSide[TAB2[validAxis]])
    return true;
  mcIdType oldAxis(ToIdType(std::distance(myNeighbours,std::find(myNeighbours,myNeighbours+6,neighId))));
  std::size_t pos(std::distance(MEDCoupling1SGTUMesh::HEXA8_FACE_PAIRS,std::find(MEDCoupling1SGTUMesh::HEXA8_FACE_PAIRS,MEDCoupling1SGTUMesh::HEXA8_FACE_PAIRS+6,oldAxis)));
  std::size_t pos0(pos/2),pos1(pos%2);
  int oldAxisOpp(MEDCoupling1SGTUMesh::HEXA8_FACE_PAIRS[2*pos0+(pos1+1)%2]);
  mcIdType oldConn[8],myConn2[8]={-1,-1,-1,-1,-1,-1,-1,-1},myConn[8],edgeConn[2],allFacesTmp[24],neighTmp[6];
  oldConn[0]=allFacesNodalConn[0]; oldConn[1]=allFacesNodalConn[1]; oldConn[2]=allFacesNodalConn[2]; oldConn[3]=allFacesNodalConn[3];
  oldConn[4]=allFacesNodalConn[4]; oldConn[5]=allFacesNodalConn[7]; oldConn[6]=allFacesNodalConn[6]; oldConn[7]=allFacesNodalConn[5];
  const INTERP_KERNEL::CellModel& cm(INTERP_KERNEL::CellModel::GetCellModel(INTERP_KERNEL::NORM_HEXA8));
  for(int i=0;i<4;i++)
    myConn2[i]=validConnQuad4NeighSide[(4-i+TAB2[validAxis])%4];
  for(int i=0;i<4;i++)
    {
      mcIdType nodeId(myConn2[i]);//the node id for which the opposite one will be found
      bool found(false);
      INTERP_KERNEL::NormalizedCellType typeOfSon;
      for(int j=0;j<12 && !found;j++)
        {
          cm.fillSonEdgesNodalConnectivity3D(j,oldConn,-1,edgeConn,typeOfSon);
          if(edgeConn[0]==nodeId || edgeConn[1]==nodeId)
            {
              if(std::find(allFacesNodalConn+4*oldAxisOpp,allFacesNodalConn+4*oldAxisOpp+4,edgeConn[0]==nodeId?edgeConn[1]:edgeConn[0])!=allFacesNodalConn+4*oldAxisOpp+4)
                {
                  myConn2[i+4]=edgeConn[0]==nodeId?edgeConn[1]:edgeConn[0];
                  found=true;
                }
            }
        }
      if(!found)
        throw INTERP_KERNEL::Exception("UpdateHexa8Cell : Internal Error !");
    }
  const int *myTab(TAB+8*validAxis);
  for(int i=0;i<8;i++)
    myConn[i]=myConn2[myTab[i]];
  for(int i=0;i<6;i++)
    {
      cm.fillSonCellNodalConnectivity(i,myConn,allFacesTmp+4*i);
      std::set<mcIdType> s(allFacesTmp+4*i,allFacesTmp+4*i+4);
      bool found(false);
      for(int j=0;j<6 && !found;j++)
        {
          std::set<mcIdType> s1(allFacesNodalConn+4*j,allFacesNodalConn+4*j+4);
          if(s==s1)
            {
              neighTmp[i]=myNeighbours[j];
              found=true;
            }
        }
      if(!found)
        throw INTERP_KERNEL::Exception("UpdateHexa8Cell : Internal Error #2 !");
    }
  std::copy(allFacesTmp,allFacesTmp+24,allFacesNodalConn);
  std::copy(neighTmp,neighTmp+6,myNeighbours);
  return false;
}

/// @endcond

/*!
 * This method expects the \a this contains NORM_HEXA8 cells only. This method will sort each cells in \a this so that their numbering was
 * homogeneous. If it succeeds the result of MEDCouplingUMesh::tetrahedrize will return a conform mesh.
 *
 * \return DataArrayIdType * - a newly allocated array (to be managed by the caller) containing renumbered cell ids.
 *
 * \throw If \a this is not a mesh containing only NORM_HEXA8 cells.
 * \throw If \a this is not properly allocated.
 * \sa MEDCouplingUMesh::tetrahedrize, MEDCouplingUMesh::simplexize.
 */
DataArrayIdType *MEDCoupling1SGTUMesh::sortHexa8EachOther()
{
  MCAuto<MEDCoupling1SGTUMesh> quads(explodeEachHexa8To6Quad4());//checks that only hexa8
  mcIdType nbHexa8=getNumberOfCells();
  mcIdType *cQuads(quads->getNodalConnectivity()->getPointer());
  MCAuto<DataArrayIdType> neighOfQuads(DataArrayIdType::New()); neighOfQuads->alloc(nbHexa8*6,1); neighOfQuads->fillWithValue(-1);
  mcIdType *ptNeigh(neighOfQuads->getPointer());
  {//neighOfQuads tells for each face of each Quad8 which cell (if!=-1) is connected to this face.
    MCAuto<MEDCouplingUMesh> quadsTmp(quads->buildUnstructured());
    MCAuto<DataArrayIdType> ccSafe,cciSafe;
    DataArrayIdType *cc(0),*cci(0);
    quadsTmp->findCommonCells(3,0,cc,cci);
    ccSafe=cc; cciSafe=cci;
    const mcIdType *ccPtr(ccSafe->begin());
    mcIdType nbOfPair=cci->getNumberOfTuples()-1;
    for(mcIdType i=0;i<nbOfPair;i++)
      { ptNeigh[ccPtr[2*i+0]]=ccPtr[2*i+1]/6; ptNeigh[ccPtr[2*i+1]]=ccPtr[2*i+0]/6; }
  }
  MCAuto<DataArrayIdType> ret(DataArrayIdType::New()); ret->alloc(0,1);
  std::vector<bool> fetched(nbHexa8,false);
  std::vector<bool>::iterator it(std::find(fetched.begin(),fetched.end(),false));
  while(it!=fetched.end())//it will turns as time as number of connected zones
    {
      mcIdType cellId(ToIdType(std::distance(fetched.begin(),it)));//it is the seed of the connected zone.
      std::set<mcIdType> s; s.insert(cellId);//s contains already organized.
      while(!s.empty())
        {
          std::set<mcIdType> sNext;
          for(std::set<mcIdType>::const_iterator it0=s.begin();it0!=s.end();it0++)
            {
              fetched[*it0]=true;
              mcIdType *myNeighb(ptNeigh+6*(*it0));
              for(int i=0;i<6;i++)
                {
                  if(myNeighb[i]!=-1 && !fetched[myNeighb[i]])
                    {
                      std::size_t pos(std::distance(HEXA8_FACE_PAIRS,std::find(HEXA8_FACE_PAIRS,HEXA8_FACE_PAIRS+6,i)));
                      std::size_t pos0(pos/2),pos1(pos%2);
                      if(!UpdateHexa8Cell(HEXA8_FACE_PAIRS[2*pos0+(pos1+1)%2],*it0,cQuads+6*4*(*it0)+4*i,cQuads+6*4*myNeighb[i],ptNeigh+6*myNeighb[i]))
                        ret->pushBackSilent(myNeighb[i]);
                      fetched[myNeighb[i]]=true;
                      sNext.insert(myNeighb[i]);
                    }
                }
            }
          s=sNext;
        }
      it=std::find(fetched.begin(),fetched.end(),false);
    }
  if(!ret->empty())
    {
      mcIdType *conn(getNodalConnectivity()->getPointer());
      for(const mcIdType *pt=ret->begin();pt!=ret->end();pt++)
        {
          mcIdType cellId(*pt);
          conn[8*cellId+0]=cQuads[24*cellId+0]; conn[8*cellId+1]=cQuads[24*cellId+1]; conn[8*cellId+2]=cQuads[24*cellId+2]; conn[8*cellId+3]=cQuads[24*cellId+3];
          conn[8*cellId+4]=cQuads[24*cellId+4]; conn[8*cellId+5]=cQuads[24*cellId+7]; conn[8*cellId+6]=cQuads[24*cellId+6]; conn[8*cellId+7]=cQuads[24*cellId+5];
        }
      declareAsNew();
    }
  return ret.retn();
}

MEDCoupling1DGTUMesh *MEDCoupling1SGTUMesh::computeDualMesh3D() const
{
  static const int DUAL_TETRA_0[36]={
    4,1,0, 6,0,3, 7,3,1,
    4,0,1, 5,2,0, 8,1,2,
    6,3,0, 5,0,2, 9,2,3,
    7,1,3, 9,3,2, 8,2,1
  };
  static const int DUAL_TETRA_1[36]={
    8,4,10, 11,5,8, 10,7,11,
    9,4,8, 8,5,12, 12,6,9,
    10,4,9, 9,6,13, 13,7,10,
    12,5,11, 13,6,12, 11,7,13
  };
  static const int FACEID_NOT_SH_NODE[4]={2,3,1,0};
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_TETRA4)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::computeDualMesh3D : only TETRA4 supported !");
  checkFullyDefined();
  MCAuto<MEDCouplingUMesh> thisu(buildUnstructured());
  MCAuto<DataArrayIdType> revNodArr(DataArrayIdType::New()),revNodIArr(DataArrayIdType::New());
  thisu->getReverseNodalConnectivity(revNodArr,revNodIArr);
  const mcIdType *revNod(revNodArr->begin()),*revNodI(revNodIArr->begin()),*nodal(_conn->begin());
  MCAuto<DataArrayIdType> d1Arr(DataArrayIdType::New()),di1Arr(DataArrayIdType::New()),rd1Arr(DataArrayIdType::New()),rdi1Arr(DataArrayIdType::New());
  MCAuto<MEDCouplingUMesh> edges(thisu->explode3DMeshTo1D(d1Arr,di1Arr,rd1Arr,rdi1Arr));
  const mcIdType *d1(d1Arr->begin());
  MCAuto<DataArrayIdType> d2Arr(DataArrayIdType::New()),di2Arr(DataArrayIdType::New()),rd2Arr(DataArrayIdType::New()),rdi2Arr(DataArrayIdType::New());
  MCAuto<MEDCouplingUMesh> faces(thisu->buildDescendingConnectivity(d2Arr,di2Arr,rd2Arr,rdi2Arr));  thisu=0;
  const mcIdType *d2(d2Arr->begin()),*rdi2(rdi2Arr->begin());
  MCAuto<DataArrayDouble> edgesBaryArr(edges->computeCellCenterOfMass()),facesBaryArr(faces->computeCellCenterOfMass()),baryArr(computeCellCenterOfMass());
  const mcIdType nbOfNodes(getNumberOfNodes());
  const mcIdType offset0=nbOfNodes+faces->getNumberOfCells();
  const mcIdType offset1=offset0+edges->getNumberOfCells();
  edges=0; faces=0;
  std::vector<const DataArrayDouble *> v(4); v[0]=getCoords(); v[1]=facesBaryArr; v[2]=edgesBaryArr; v[3]=baryArr;
  MCAuto<DataArrayDouble> zeArr(DataArrayDouble::Aggregate(v)); baryArr=0; edgesBaryArr=0; facesBaryArr=0;
  std::string name("DualOf_"); name+=getName();
  MCAuto<MEDCoupling1DGTUMesh> ret(MEDCoupling1DGTUMesh::New(name,INTERP_KERNEL::NORM_POLYHED)); ret->setCoords(zeArr);
  MCAuto<DataArrayIdType> cArr(DataArrayIdType::New()),ciArr(DataArrayIdType::New()); ciArr->alloc(nbOfNodes+1,1); ciArr->setIJ(0,0,0); cArr->alloc(0,1);
  for(mcIdType i=0;i<nbOfNodes;i++,revNodI++)
    {
      mcIdType nbOfCellsSharingNode(revNodI[1]-revNodI[0]);
      if(nbOfCellsSharingNode==0)
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::computeDualMesh3D : Node #" << i << " is orphan !"; 
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
      for(int j=0;j<nbOfCellsSharingNode;j++)
        {
          mcIdType curCellId(revNod[revNodI[0]+j]);
          const mcIdType *connOfCurCell(nodal+4*curCellId);
          std::size_t nodePosInCurCell(std::distance(connOfCurCell,std::find(connOfCurCell,connOfCurCell+4,i)));
          if(j!=0) cArr->pushBackSilent(-1);
          mcIdType tmp[14];
          //
          tmp[0]=d1[6*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+0]-4]+offset0; tmp[1]=d2[4*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+1]]+nbOfNodes;
          tmp[2]=curCellId+offset1; tmp[3]=d2[4*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+2]]+nbOfNodes;
          tmp[4]=-1;
          tmp[5]=d1[6*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+3]-4]+offset0; tmp[6]=d2[4*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+4]]+nbOfNodes;
          tmp[7]=curCellId+offset1; tmp[8]=d2[4*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+5]]+nbOfNodes;
          tmp[9]=-1;
          tmp[10]=d1[6*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+6]-4]+offset0; tmp[11]=d2[4*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+7]]+nbOfNodes;
          tmp[12]=curCellId+offset1; tmp[13]=d2[4*curCellId+DUAL_TETRA_0[nodePosInCurCell*9+8]]+nbOfNodes;
          cArr->insertAtTheEnd(tmp,tmp+14);
          int kk(0);
          for(int k=0;k<4;k++)
            {
              if(FACEID_NOT_SH_NODE[nodePosInCurCell]!=k)
                {
                  const mcIdType *faceId(d2+4*curCellId+k);
                  if(rdi2[*faceId+1]-rdi2[*faceId]==1)
                    {
                      mcIdType tmp2[5]; tmp2[0]=-1; tmp2[1]=i;
                      tmp2[2]=d1[6*curCellId+DUAL_TETRA_1[9*nodePosInCurCell+3*kk+0]-8]+offset0;
                      tmp2[3]=d2[4*curCellId+DUAL_TETRA_1[9*nodePosInCurCell+3*kk+1]-4]+nbOfNodes;
                      tmp2[4]=d1[6*curCellId+DUAL_TETRA_1[9*nodePosInCurCell+3*kk+2]-8]+offset0;
                      cArr->insertAtTheEnd(tmp2,tmp2+5);
                    }
                  kk++;
                }
            }
        }
      ciArr->setIJ(i+1,0,cArr->getNumberOfTuples());
    }
  ret->setNodalConnectivity(cArr,ciArr);
  return ret.retn();
}

MEDCoupling1DGTUMesh *MEDCoupling1SGTUMesh::computeDualMesh2D() const
{
  static const int DUAL_TRI_0[6]={0,2, 1,0, 2,1};
  static const int DUAL_TRI_1[6]={-3,+5, +3,-4, +4,-5};
  static const int FACEID_NOT_SH_NODE[3]={1,2,0};
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_TRI3)
    throw INTERP_KERNEL::Exception("MEDCoupling1SGTUMesh::computeDualMesh2D : only TRI3 supported !");
  checkFullyDefined();
  MCAuto<MEDCouplingUMesh> thisu(buildUnstructured());
  MCAuto<DataArrayIdType> revNodArr(DataArrayIdType::New()),revNodIArr(DataArrayIdType::New());
  thisu->getReverseNodalConnectivity(revNodArr,revNodIArr);
  const mcIdType *revNod(revNodArr->begin()),*revNodI(revNodIArr->begin()),*nodal(_conn->begin());
  MCAuto<DataArrayIdType> d2Arr(DataArrayIdType::New()),di2Arr(DataArrayIdType::New()),rd2Arr(DataArrayIdType::New()),rdi2Arr(DataArrayIdType::New());
  MCAuto<MEDCouplingUMesh> edges(thisu->buildDescendingConnectivity(d2Arr,di2Arr,rd2Arr,rdi2Arr));  thisu=0;
  const mcIdType *d2(d2Arr->begin()),*rdi2(rdi2Arr->begin());
  MCAuto<DataArrayDouble> edgesBaryArr(edges->computeCellCenterOfMass()),baryArr(computeCellCenterOfMass());
  const mcIdType nbOfNodes(getNumberOfNodes()),offset0(nbOfNodes+edges->getNumberOfCells());
  edges=0;
  std::vector<const DataArrayDouble *> v(3); v[0]=getCoords(); v[1]=edgesBaryArr; v[2]=baryArr;
  MCAuto<DataArrayDouble> zeArr(DataArrayDouble::Aggregate(v)); baryArr=0; edgesBaryArr=0;
  std::string name("DualOf_"); name+=getName();
  MCAuto<MEDCoupling1DGTUMesh> ret(MEDCoupling1DGTUMesh::New(name,INTERP_KERNEL::NORM_POLYGON)); ret->setCoords(zeArr);
  MCAuto<DataArrayIdType> cArr(DataArrayIdType::New()),ciArr(DataArrayIdType::New()); ciArr->alloc(nbOfNodes+1,1); ciArr->setIJ(0,0,0); cArr->alloc(0,1);
  for(mcIdType i=0;i<nbOfNodes;i++,revNodI++)
    {
      mcIdType nbOfCellsSharingNode(revNodI[1]-revNodI[0]);
      if(nbOfCellsSharingNode==0)
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::computeDualMesh2D : Node #" << i << " is orphan !"; 
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
      std::vector< std::vector<mcIdType> > polyg;
      for(int j=0;j<nbOfCellsSharingNode;j++)
        {
          mcIdType curCellId(revNod[revNodI[0]+j]);
          const mcIdType *connOfCurCell(nodal+3*curCellId);
          std::size_t nodePosInCurCell(std::distance(connOfCurCell,std::find(connOfCurCell,connOfCurCell+4,i)));
          std::vector<mcIdType> locV(3);
          locV[0]=d2[3*curCellId+DUAL_TRI_0[2*nodePosInCurCell+0]]+nbOfNodes; locV[1]=curCellId+offset0; locV[2]=d2[3*curCellId+DUAL_TRI_0[2*nodePosInCurCell+1]]+nbOfNodes;
          polyg.push_back(locV);
          int kk(0);
          for(int k=0;k<3;k++)
            {
              if(FACEID_NOT_SH_NODE[nodePosInCurCell]!=k)
                {
                  const mcIdType *edgeId(d2+3*curCellId+k);
                  if(rdi2[*edgeId+1]-rdi2[*edgeId]==1)
                    {
                      std::vector<mcIdType> locV2(2);
                      int zeLocEdgeIdRel(DUAL_TRI_1[2*nodePosInCurCell+kk]);
                      if(zeLocEdgeIdRel>0)
                        {  locV2[0]=d2[3*curCellId+zeLocEdgeIdRel-3]+nbOfNodes;  locV2[1]=i; }
                      else
                        {  locV2[0]=i; locV2[1]=d2[3*curCellId-zeLocEdgeIdRel-3]+nbOfNodes; }
                      polyg.push_back(locV2);
                    }
                  kk++;
                }
            }
        }
      std::vector<mcIdType> zePolyg(MEDCoupling1DGTUMesh::BuildAPolygonFromParts(polyg));
      cArr->insertAtTheEnd(zePolyg.begin(),zePolyg.end());
      ciArr->setIJ(i+1,0,cArr->getNumberOfTuples());
    }
  ret->setNodalConnectivity(cArr,ciArr);
  return ret.retn();
}

/*!
 * This method aggregate the bbox of each cell and put it into bbox 
 *
 * \param [in] arcDetEps - a parameter specifying in case of 2D quadratic polygon cell the detection limit between linear and arc circle. (By default 1e-12)
 *                         For all other cases this input parameter is ignored.
 * \return DataArrayDouble * - newly created object (to be managed by the caller) \a this number of cells tuples and 2*spacedim components.
 * 
 * \throw If \a this is not fully set (coordinates and connectivity).
 * \throw If a cell in \a this has no valid nodeId.
 */
DataArrayDouble *MEDCoupling1SGTUMesh::getBoundingBoxForBBTree(double arcDetEps) const
{
  mcIdType spaceDim(getSpaceDimension()),nbOfCells(getNumberOfCells()),nbOfNodes(getNumberOfNodes()),nbOfNodesPerCell(getNumberOfNodesPerCell());
  MCAuto<DataArrayDouble> ret(DataArrayDouble::New()); ret->alloc(nbOfCells,2*spaceDim);
  double *bbox(ret->getPointer());
  for(mcIdType i=0;i<nbOfCells*spaceDim;i++)
    {
      bbox[2*i]=std::numeric_limits<double>::max();
      bbox[2*i+1]=-std::numeric_limits<double>::max();
    }
  const double *coordsPtr(_coords->getConstPointer());
  const mcIdType *conn(_conn->getConstPointer());
  for(mcIdType i=0;i<nbOfCells;i++)
    {
      int kk(0);
      for(int j=0;j<nbOfNodesPerCell;j++,conn++)
        {
          mcIdType nodeId(*conn);
          if(nodeId>=0 && nodeId<nbOfNodes)
            {
              for(int k=0;k<spaceDim;k++)
                {
                  bbox[2*spaceDim*i+2*k]=std::min(bbox[2*spaceDim*i+2*k],coordsPtr[spaceDim*nodeId+k]);
                  bbox[2*spaceDim*i+2*k+1]=std::max(bbox[2*spaceDim*i+2*k+1],coordsPtr[spaceDim*nodeId+k]);
                }
              kk++;
            }
        }
      if(kk==0)
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::getBoundingBoxForBBTree : cell #" << i << " contains no valid nodeId !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  return ret.retn();
}

/*!
 * Returns the cell field giving for each cell in \a this its diameter. Diameter means the max length of all possible SEG2 in the cell.
 *
 * \return a new instance of field containing the result. The returned instance has to be deallocated by the caller.
 */
MEDCouplingFieldDouble *MEDCoupling1SGTUMesh::computeDiameterField() const
{
  checkFullyDefined();
  MCAuto<MEDCouplingFieldDouble> ret(MEDCouplingFieldDouble::New(ON_CELLS,ONE_TIME));
  mcIdType nbCells=getNumberOfCells();
  MCAuto<DataArrayDouble> arr(DataArrayDouble::New());
  arr->alloc(nbCells,1);
  INTERP_KERNEL::AutoCppPtr<INTERP_KERNEL::DiameterCalculator> dc(_cm->buildInstanceOfDiameterCalulator(getSpaceDimension()));
  dc->computeFor1SGTUMeshFrmt(nbCells,_conn->begin(),getCoords()->begin(),arr->getPointer());
  ret->setMesh(this);
  ret->setArray(arr);
  ret->setName("Diameter");
  return ret.retn();
}

/*!
 * This method invert orientation of all cells in \a this. 
 * After calling this method the absolute value of measure of cells in \a this are the same than before calling.
 * This method only operates on the connectivity so coordinates are not touched at all.
 */
void MEDCoupling1SGTUMesh::invertOrientationOfAllCells()
{
  checkConsistencyOfConnectivity();
  INTERP_KERNEL::AutoCppPtr<INTERP_KERNEL::OrientationInverter> oi(INTERP_KERNEL::OrientationInverter::BuildInstanceFrom(getCellModelEnum()));
  mcIdType nbOfNodesPerCell=ToIdType(_cm->getNumberOfNodes()),nbCells=getNumberOfCells();
  mcIdType *conn(_conn->getPointer());
  for(mcIdType i=0;i<nbCells;i++)
    oi->operate(conn+i*nbOfNodesPerCell,conn+(i+1)*nbOfNodesPerCell);
  updateTime();
}

//== 

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::New()
{
  return new MEDCoupling1DGTUMesh;
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::New(const std::string& name, INTERP_KERNEL::NormalizedCellType type)
{
  if(type==INTERP_KERNEL::NORM_ERROR)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::New : NORM_ERROR is not a valid type to be used as base geometric type for a mesh !");
  const INTERP_KERNEL::CellModel& cm=INTERP_KERNEL::CellModel::GetCellModel(type);
  if(!cm.isDynamic())
    {
      std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::New : the input geometric type " << cm.getRepr() << " is static ! Only dynamic types are allowed here !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
  return new MEDCoupling1DGTUMesh(name,cm);
}

MEDCoupling1DGTUMesh::MEDCoupling1DGTUMesh()
{
}

MEDCoupling1DGTUMesh::MEDCoupling1DGTUMesh(const std::string& name, const INTERP_KERNEL::CellModel& cm):MEDCoupling1GTUMesh(name,cm)
{
}

MEDCoupling1DGTUMesh::MEDCoupling1DGTUMesh(const MEDCoupling1DGTUMesh& other, bool recDeepCpy):MEDCoupling1GTUMesh(other,recDeepCpy),_conn_indx(other._conn_indx),_conn(other._conn)
{
  if(recDeepCpy)
    {
      const DataArrayIdType *c(other._conn);
      if(c)
        _conn=c->deepCopy();
      c=other._conn_indx;
      if(c)
        _conn_indx=c->deepCopy();
    }
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::clone(bool recDeepCpy) const
{
  return new MEDCoupling1DGTUMesh(*this,recDeepCpy);
}

/*!
 * This method behaves mostly like MEDCoupling1DGTUMesh::deepCopy method, except that only nodal connectivity arrays are deeply copied.
 * The coordinates are shared between \a this and the returned instance.
 * 
 * \return MEDCoupling1DGTUMesh * - A new object instance holding the copy of \a this (deep for connectivity, shallow for coordiantes)
 * \sa MEDCoupling1DGTUMesh::deepCopy
 */
MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::deepCopyConnectivityOnly() const
{
  checkConsistencyLight();
  MCAuto<MEDCoupling1DGTUMesh> ret(clone(false));
  MCAuto<DataArrayIdType> c(_conn->deepCopy()),ci(_conn_indx->deepCopy());
  ret->setNodalConnectivity(c,ci);
  return ret.retn();
}

void MEDCoupling1DGTUMesh::updateTime() const
{
  MEDCoupling1GTUMesh::updateTime();
  const DataArrayIdType *c(_conn);
  if(c)
    updateTimeWith(*c);
  c=_conn_indx;
  if(c)
    updateTimeWith(*c);
}

std::size_t MEDCoupling1DGTUMesh::getHeapMemorySizeWithoutChildren() const
{
  return MEDCoupling1GTUMesh::getHeapMemorySizeWithoutChildren();
}

std::vector<const BigMemoryObject *> MEDCoupling1DGTUMesh::getDirectChildrenWithNull() const
{
  std::vector<const BigMemoryObject *> ret(MEDCoupling1GTUMesh::getDirectChildrenWithNull());
  ret.push_back((const DataArrayIdType *)_conn);
  ret.push_back((const DataArrayIdType *)_conn_indx);
  return ret;
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::deepCopy() const
{
  return clone(true);
}

bool MEDCoupling1DGTUMesh::isEqualIfNotWhy(const MEDCouplingMesh *other, double prec, std::string& reason) const
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::isEqualIfNotWhy : input other pointer is null !");
  std::ostringstream oss; oss.precision(15);
  const MEDCoupling1DGTUMesh *otherC=dynamic_cast<const MEDCoupling1DGTUMesh *>(other);
  if(!otherC)
    {
      reason="mesh given in input is not castable in MEDCoupling1DGTUMesh !";
      return false;
    }
  if(!MEDCoupling1GTUMesh::isEqualIfNotWhy(other,prec,reason))
    return false;
  const DataArrayIdType *c1(_conn),*c2(otherC->_conn);
  if(c1==c2)
    return true;
  if(!c1 || !c2)
    {
      reason="in connectivity of single dynamic geometric type exactly one among this and other is null !";
      return false;
    }
  if(!c1->isEqualIfNotWhy(*c2,reason))
    {
      reason.insert(0,"Nodal connectivity DataArrayIdType differs : ");
      return false;
    }
  c1=_conn_indx; c2=otherC->_conn_indx;
  if(c1==c2)
    return true;
  if(!c1 || !c2)
    {
      reason="in connectivity index of single dynamic geometric type exactly one among this and other is null !";
      return false;
    }
  if(!c1->isEqualIfNotWhy(*c2,reason))
    {
      reason.insert(0,"Nodal connectivity index DataArrayIdType differs : ");
      return false;
    }
  return true;
}

bool MEDCoupling1DGTUMesh::isEqualWithoutConsideringStr(const MEDCouplingMesh *other, double prec) const
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::isEqualWithoutConsideringStr : input other pointer is null !");
  const MEDCoupling1DGTUMesh *otherC=dynamic_cast<const MEDCoupling1DGTUMesh *>(other);
  if(!otherC)
    return false;
  if(!MEDCoupling1GTUMesh::isEqualWithoutConsideringStr(other,prec))
    return false;
  const DataArrayIdType *c1(_conn),*c2(otherC->_conn);
  if(c1==c2)
    return true;
  if(!c1 || !c2)
    return false;
  if(!c1->isEqualWithoutConsideringStr(*c2))
    return false;
  return true;
}

/*!
 * Checks if \a this and \a other meshes are geometrically equivalent with high
 * probability, else an exception is thrown. The meshes are considered equivalent if
 * (1) meshes contain the same number of nodes and the same number of elements of the
 * same types (2) three cells of the two meshes (first, last and middle) are based
 * on coincident nodes (with a specified precision).
 *  \param [in] other - the mesh to compare with.
 *  \param [in] prec - the precision used to compare nodes of the two meshes.
 *  \throw If the two meshes do not match.
 */
void MEDCoupling1DGTUMesh::checkFastEquivalWith(const MEDCouplingMesh *other, double prec) const
{
  MEDCouplingPointSet::checkFastEquivalWith(other,prec);
  const MEDCoupling1DGTUMesh *otherC=dynamic_cast<const MEDCoupling1DGTUMesh *>(other);
  if(!otherC)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : Two meshes are not unstructured with single dynamic geometric type !");
  const DataArrayIdType *c1(_conn),*c2(otherC->_conn);
  if(c1!=c2)
    {
      if(!c1 || !c2)
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : presence of nodal connectivity only in one of the 2 meshes !");
      if((c1->isAllocated() && !c2->isAllocated()) || (!c1->isAllocated() && c2->isAllocated()))
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : in nodal connectivity, only one is allocated !");
      if(c1->getNumberOfComponents()!=1 || c1->getNumberOfComponents()!=1)
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : in nodal connectivity, must have 1 and only 1 component !");
      if(c1->getHashCode()!=c2->getHashCode())
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : nodal connectivity differs");
    }
  c1=_conn_indx; c2=otherC->_conn_indx;
  if(c1!=c2)
    {
      if(!c1 || !c2)
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : presence of nodal connectivity index only in one of the 2 meshes !");
      if((c1->isAllocated() && !c2->isAllocated()) || (!c1->isAllocated() && c2->isAllocated()))
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : in nodal connectivity index, only one is allocated !");
      if(c1->getNumberOfComponents()!=1 || c1->getNumberOfComponents()!=1)
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : in nodal connectivity index, must have 1 and only 1 component !");
      if(c1->getHashCode()!=c2->getHashCode())
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFastEquivalWith : nodal connectivity index differs");
    }
}

void MEDCoupling1DGTUMesh::checkConsistencyOfConnectivity() const
{
  const DataArrayIdType *c1(_conn);
  if(c1)
    {
      if(c1->getNumberOfComponents()!=1)
        throw INTERP_KERNEL::Exception("Nodal connectivity array is expected to be with number of components set to one !");
      if(c1->getInfoOnComponent(0)!="")
        throw INTERP_KERNEL::Exception("Nodal connectivity array is expected to have no info on its single component !");
      c1->checkAllocated();
    }
  else
    throw INTERP_KERNEL::Exception("Nodal connectivity array not defined !");
  //
  mcIdType sz2(_conn->getNumberOfTuples());
  c1=_conn_indx;
  if(c1)
    {
      if(c1->getNumberOfComponents()!=1)
        throw INTERP_KERNEL::Exception("Nodal connectivity index array is expected to be with number of components set to one !");
      c1->checkAllocated();
      if(c1->getNumberOfTuples()<1)
        throw INTERP_KERNEL::Exception("Nodal connectivity index array is expected to have a a size of 1 at least !");
      if(c1->getInfoOnComponent(0)!="")
        throw INTERP_KERNEL::Exception("Nodal connectivity index array is expected to have no info on its single component !");
      mcIdType f=c1->front(),ll=c1->back();
      if(f<0 || (sz2>0 && f>=sz2))
        {
          std::ostringstream oss; oss << "Nodal connectivity index array first value (" << f << ") is expected to be exactly in [0," << sz2 << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
      if(ll<0 || ll>sz2)
        {
          std::ostringstream oss; oss << "Nodal connectivity index array last value (" << ll << ") is expected to be exactly in [0," << sz2 << "] !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
      if(f>ll)
        {
          std::ostringstream oss; oss << "Nodal connectivity index array looks very bad (not increasing monotonic) because front (" << f << ") is greater that back (" << ll << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  else
    throw INTERP_KERNEL::Exception("Nodal connectivity index array not defined !");
  mcIdType szOfC1Exp=_conn_indx->back();
  if(sz2<szOfC1Exp)
    {
      std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::checkConsistencyOfConnectivity : The expected length of nodal connectivity array regarding index is " << szOfC1Exp << " but the actual size of it is " << c1->getNumberOfTuples() << " !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
}

/*!
 * If \a this pass this method, you are sure that connectivity arrays are not null, with exactly one component, no name, no component name, allocated.
 * In addition you are sure that the length of nodal connectivity index array is bigger than or equal to one.
 * In addition you are also sure that length of nodal connectivity is coherent with the content of the last value in the index array.
 */
void MEDCoupling1DGTUMesh::checkConsistencyLight() const
{
  MEDCouplingPointSet::checkConsistencyLight();
  checkConsistencyOfConnectivity();
}

void MEDCoupling1DGTUMesh::checkConsistency(double eps) const
{
  checkConsistencyLight();
  const DataArrayIdType *c1(_conn),*c2(_conn_indx);
  if(!c2->isMonotonic(true))
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkConsistency : the nodal connectivity index is expected to be increasing monotinic !");
  //
  mcIdType nbOfTuples(c1->getNumberOfTuples());
  mcIdType nbOfNodes=getNumberOfNodes();
  const mcIdType *w(c1->begin());
  for(mcIdType i=0;i<nbOfTuples;i++,w++)
    {
      if(*w==-1) continue;
      if(*w<0 || *w>=nbOfNodes)
        {
          std::ostringstream oss; oss << "At pos #" << i << " of nodal connectivity array references to node id #" << *w << " must be in [0," << nbOfNodes << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
}

mcIdType MEDCoupling1DGTUMesh::getNumberOfCells() const
{
  checkConsistencyOfConnectivity();//do not remove
  return _conn_indx->getNumberOfTuples()-1;
}

/*!
 * This method returns a newly allocated array containing this->getNumberOfCells() tuples and 1 component.
 * For each cell in \b this the number of nodes constituting cell is computed.
 * For each polyhedron cell, the sum of the number of nodes of each face constituting polyhedron cell is returned.
 * So for pohyhedrons some nodes can be counted several times in the returned result.
 * 
 * \return a newly allocated array
 */
DataArrayIdType *MEDCoupling1DGTUMesh::computeNbOfNodesPerCell() const
{
  checkConsistencyLight();
  _conn_indx->checkMonotonic(true);
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_POLYHED)
    return _conn_indx->deltaShiftIndex();
  // for polyhedrons
  mcIdType nbOfCells=_conn_indx->getNumberOfTuples()-1;
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(nbOfCells,1);
  mcIdType *retPtr=ret->getPointer();
  const mcIdType *ci=_conn_indx->begin(),*c=_conn->begin();
  for(mcIdType i=0;i<nbOfCells;i++,retPtr++,ci++)
    *retPtr=int(ci[1]-ci[0]-ToIdType(std::count(c+ci[0],c+ci[1],-1)));
  return ret.retn();
}

/*!
 * This method returns a newly allocated array containing this->getNumberOfCells() tuples and 1 component.
 * For each cell in \b this the number of faces constituting (entity of dimension this->getMeshDimension()-1) cell is computed.
 * 
 * \return a newly allocated array
 */
DataArrayIdType *MEDCoupling1DGTUMesh::computeNbOfFacesPerCell() const
{
  checkConsistencyLight();
  _conn_indx->checkMonotonic(true);
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_POLYHED && getCellModelEnum()!=INTERP_KERNEL::NORM_QPOLYG)
    return _conn_indx->deltaShiftIndex();
  if(getCellModelEnum()==INTERP_KERNEL::NORM_QPOLYG)
    {
      MCAuto<DataArrayIdType> ret=_conn_indx->deltaShiftIndex();
      ret->applyDivideBy(2);
      return ret.retn();
    }
  // for polyhedrons
  mcIdType nbOfCells=_conn_indx->getNumberOfTuples()-1;
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(nbOfCells,1);
  mcIdType *retPtr=ret->getPointer();
  const mcIdType *ci=_conn_indx->begin(),*c=_conn->begin();
  for(mcIdType i=0;i<nbOfCells;i++,retPtr++,ci++)
    *retPtr=ToIdType(std::count(c+ci[0],c+ci[1],-1))+1;
  return ret.retn();
}

/*!
 * This method computes effective number of nodes per cell. That is to say nodes appearing several times in nodal connectivity of a cell,
 * will be counted only once here whereas it will be counted several times in MEDCoupling1DGTUMesh::computeNbOfNodesPerCell method.
 *
 * \return DataArrayIdType * - new object to be deallocated by the caller.
 * \sa MEDCoupling1DGTUMesh::computeNbOfNodesPerCell
 */
DataArrayIdType *MEDCoupling1DGTUMesh::computeEffectiveNbOfNodesPerCell() const
{
  checkConsistencyLight();
  _conn_indx->checkMonotonic(true);
  mcIdType nbOfCells=_conn_indx->getNumberOfTuples()-1;
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(nbOfCells,1);
  mcIdType *retPtr(ret->getPointer());
  const mcIdType *ci(_conn_indx->begin()),*c(_conn->begin());
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_POLYHED)
    {
      for(mcIdType i=0;i<nbOfCells;i++,retPtr++,ci++)
        {
          std::set<mcIdType> s(c+ci[0],c+ci[1]);
          *retPtr=ToIdType(s.size());
        }
    }
  else
    {
      for(mcIdType i=0;i<nbOfCells;i++,retPtr++,ci++)
        {
          std::set<mcIdType> s(c+ci[0],c+ci[1]); s.erase(-1);
          *retPtr=ToIdType(s.size());
        }
    }
  return ret.retn();
}

void MEDCoupling1DGTUMesh::getNodeIdsOfCell(mcIdType cellId, std::vector<mcIdType>& conn) const
{
  mcIdType nbOfCells(getNumberOfCells());//performs checks
  if(cellId<nbOfCells)
    {
      mcIdType strt=_conn_indx->getIJ(cellId,0),stp=_conn_indx->getIJ(cellId+1,0);
      mcIdType nbOfNodes=stp-strt;
      if(nbOfNodes<0)
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::getNodeIdsOfCell : the index array is invalid ! Should be increasing monotonic !");
      conn.resize(nbOfNodes);
      std::copy(_conn->begin()+strt,_conn->begin()+stp,conn.begin());
    }
  else
    {
      std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::getNodeIdsOfCell : request for cellId #" << cellId << " must be in [0," << nbOfCells << ") !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
}

mcIdType MEDCoupling1DGTUMesh::getNumberOfNodesInCell(mcIdType cellId) const
{
  mcIdType nbOfCells=getNumberOfCells();//performs checks
  if(cellId>=0 && cellId<nbOfCells)
    {
      const mcIdType *conn(_conn->begin());
      mcIdType strt=_conn_indx->getIJ(cellId,0),stp=_conn_indx->getIJ(cellId+1,0);
      return stp-strt-ToIdType(std::count(conn+strt,conn+stp,-1));
    }
  else
    {
      std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::getNumberOfNodesInCell : request for cellId #" << cellId << " must be in [0," << nbOfCells << ") !";
      throw INTERP_KERNEL::Exception(oss.str().c_str());
    }
}

std::string MEDCoupling1DGTUMesh::simpleRepr() const
{
  static const char msg0[]="No coordinates specified !";
  if(!_cm)
    return std::string("Cell type not specified");
  std::ostringstream ret;
  ret << "Single dynamic geometic type (" << _cm->getRepr() << ") unstructured mesh with name : \"" << getName() << "\"\n";
  ret << "Description of mesh : \"" << getDescription() << "\"\n";
  int tmpp1,tmpp2;
  double tt=getTime(tmpp1,tmpp2);
  ret << "Time attached to the mesh [unit] : " << tt << " [" << getTimeUnit() << "]\n";
  ret << "Iteration : " << tmpp1  << " Order : " << tmpp2 << "\n";
  ret << "Mesh dimension : " << getMeshDimension() << "\nSpace dimension : ";
  if(_coords!=0)
    {
      const int spaceDim=getSpaceDimension();
      ret << spaceDim << "\nInfo attached on space dimension : ";
      for(int i=0;i<spaceDim;i++)
        ret << "\"" << _coords->getInfoOnComponent(i) << "\" ";
      ret << "\n";
    }
  else
    ret << msg0 << "\n";
  ret << "Number of nodes : ";
  if(_coords!=0)
    ret << getNumberOfNodes() << "\n";
  else
    ret << msg0 << "\n";
  ret << "Number of cells : ";
  bool isOK=true;
  try { checkConsistencyLight(); } catch(INTERP_KERNEL::Exception& /* e */)
  {
      ret << "Nodal connectivity arrays are not set or badly set !\n";
      isOK=false;
  }
  if(isOK)
    ret << getNumberOfCells() << "\n";
  ret << "Cell type : " << _cm->getRepr() << "\n";
  return ret.str();
}

std::string MEDCoupling1DGTUMesh::advancedRepr() const
{
  std::ostringstream ret;
  ret << simpleRepr();
  ret << "\nCoordinates array : \n___________________\n\n";
  if(_coords)
    _coords->reprWithoutNameStream(ret);
  else
    ret << "No array set !\n";
  ret << "\n\nNodal Connectivity : \n____________________\n\n";
  //
  bool isOK=true;
  try { checkConsistency(); } catch(INTERP_KERNEL::Exception& /* e */)
  {
      ret << "Nodal connectivity arrays are not set or badly set !\n";
      isOK=false;
  }
  if(!isOK)
    return ret.str();
  mcIdType nbOfCells=getNumberOfCells();
  const mcIdType *ci=_conn_indx->begin(),*c=_conn->begin();
  for(mcIdType i=0;i<nbOfCells;i++,ci++)
    {
      ret << "Cell #" << i << " : ";
      std::copy(c+ci[0],c+ci[1],std::ostream_iterator<int>(ret," "));
      ret << "\n";
    }
  return ret.str();
}

DataArrayDouble *MEDCoupling1DGTUMesh::computeIsoBarycenterOfNodesPerCell() const
{
  MCAuto<DataArrayDouble> ret=DataArrayDouble::New();
  int spaceDim=getSpaceDimension();
  mcIdType nbOfCells=getNumberOfCells();//checkConsistencyLight()
  mcIdType nbOfNodes=getNumberOfNodes();
  ret->alloc(nbOfCells,spaceDim);
  double *ptToFill=ret->getPointer();
  const double *coor=_coords->begin();
  const mcIdType *nodal=_conn->begin(),*nodali=_conn_indx->begin();
  nodal+=nodali[0];
  if(getCellModelEnum()!=INTERP_KERNEL::NORM_POLYHED)
    {
      for(mcIdType i=0;i<nbOfCells;i++,ptToFill+=spaceDim,nodali++)
        {
          std::fill(ptToFill,ptToFill+spaceDim,0.);
          if(nodali[0]<nodali[1])// >= to avoid division by 0.
            {
              for(mcIdType j=nodali[0];j<nodali[1];j++,nodal++)
                {
                  if(*nodal>=0 && *nodal<nbOfNodes)
                    std::transform(coor+spaceDim*nodal[0],coor+spaceDim*(nodal[0]+1),ptToFill,ptToFill,std::plus<double>());
                  else
                    {
                      std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::computeIsoBarycenterOfNodesPerCell : on cell #" << i << " presence of nodeId #" << *nodal << " should be in [0," <<   nbOfNodes << ") !";
                      throw INTERP_KERNEL::Exception(oss.str().c_str());
                    }
                  std::transform(ptToFill,ptToFill+spaceDim,ptToFill,std::bind(std::multiplies<double>(),std::placeholders::_1,1./double(nodali[1]-nodali[0])));
                }
            }
          else
            {
              std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::computeIsoBarycenterOfNodesPerCell : at cell #" << i << " the nodal index array is invalid !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
    }
  else
    {
      for(mcIdType i=0;i<nbOfCells;i++,ptToFill+=spaceDim,nodali++)
        {
          std::fill(ptToFill,ptToFill+spaceDim,0.);
          if(nodali[0]<nodali[1])// >= to avoid division by 0.
            {
              int nbOfNod=0;
              for(mcIdType j=nodali[0];j<nodali[1];j++,nodal++)
                {
                  if(*nodal==-1) continue;
                  if(*nodal>=0 && *nodal<nbOfNodes)
                    {
                      std::transform(coor+spaceDim*nodal[0],coor+spaceDim*(nodal[0]+1),ptToFill,ptToFill,std::plus<double>());
                      nbOfNod++;
                    }
                  else
                    {
                      std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::computeIsoBarycenterOfNodesPerCell (polyhedron) : on cell #" << i << " presence of nodeId #" << *nodal << " should be in [0," <<   nbOfNodes << ") !";
                      throw INTERP_KERNEL::Exception(oss.str().c_str());
                    }
                }
              if(nbOfNod!=0)
                std::transform(ptToFill,ptToFill+spaceDim,ptToFill,std::bind(std::multiplies<double>(),std::placeholders::_1,1./nbOfNod));
              else
                {
                  std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::computeIsoBarycenterOfNodesPerCell (polyhedron) : no nodes in cell #" << i << " !";
                  throw INTERP_KERNEL::Exception(oss.str().c_str());
                }
            }
          else
            {
              std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::computeIsoBarycenterOfNodesPerCell (polyhedron)  : at cell #" << i << " the nodal index array is invalid !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
    }
  return ret.retn();
}

void MEDCoupling1DGTUMesh::renumberCells(const mcIdType *old2NewBg, bool check)
{
  mcIdType nbCells=getNumberOfCells();
  MCAuto<DataArrayIdType> o2n=DataArrayIdType::New();
  o2n->useArray(old2NewBg,false,DeallocType::C_DEALLOC,nbCells,1);
  if(check)
    o2n=o2n->checkAndPreparePermutation();
  //
  const mcIdType *o2nPtr=o2n->getPointer();
  const mcIdType *conn=_conn->begin(),*conni=_conn_indx->begin();
  MCAuto<DataArrayIdType> newConn=DataArrayIdType::New();
  MCAuto<DataArrayIdType> newConnI=DataArrayIdType::New();
  newConn->alloc(_conn->getNumberOfTuples(),1); newConnI->alloc(nbCells,1);
  newConn->copyStringInfoFrom(*_conn); newConnI->copyStringInfoFrom(*_conn_indx);
  //
  mcIdType *newC=newConn->getPointer(),*newCI=newConnI->getPointer();
  for(mcIdType i=0;i<nbCells;i++)
    {
      mcIdType newPos=o2nPtr[i];
      mcIdType sz=conni[i+1]-conni[i];
      if(sz>=0)
        newCI[newPos]=sz;
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::renumberCells : the index nodal array is invalid for cell #" << i << " !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  newConnI->computeOffsetsFull(); newCI=newConnI->getPointer();
  //
  for(mcIdType i=0;i<nbCells;i++,conni++)
    {
      mcIdType newp=o2nPtr[i];
      std::copy(conn+conni[0],conn+conni[1],newC+newCI[newp]);
    }
  _conn=newConn;
  _conn_indx=newConnI;
}

MEDCouplingMesh *MEDCoupling1DGTUMesh::mergeMyselfWith(const MEDCouplingMesh *other) const
{
  if(other->getType()!=SINGLE_DYNAMIC_GEO_TYPE_UNSTRUCTURED)
    throw INTERP_KERNEL::Exception("Merge of umesh only available with umesh single dynamic geo type each other !");
  const MEDCoupling1DGTUMesh *otherC=static_cast<const MEDCoupling1DGTUMesh *>(other);
  return Merge1DGTUMeshes(this,otherC);
}

MEDCouplingUMesh *MEDCoupling1DGTUMesh::buildUnstructured() const
{
  MCAuto<MEDCouplingUMesh> ret=MEDCouplingUMesh::New(getName(),getMeshDimension());
  ret->setCoords(getCoords());
  const mcIdType *nodalConn=_conn->begin(),*nodalConnI=_conn_indx->begin();
  mcIdType nbCells=getNumberOfCells();//checkConsistencyLight
  mcIdType geoType=ToIdType(getCellModelEnum());
  MCAuto<DataArrayIdType> c=DataArrayIdType::New(); c->alloc(nbCells+_conn->getNumberOfTuples(),1);
  MCAuto<DataArrayIdType> cI=DataArrayIdType::New(); cI->alloc(nbCells+1);
  mcIdType *cPtr=c->getPointer(),*ciPtr=cI->getPointer();
  ciPtr[0]=0;
  for(mcIdType i=0;i<nbCells;i++,ciPtr++)
    {
      mcIdType sz=nodalConnI[i+1]-nodalConnI[i];
      if(sz>=0)
        {
          *cPtr++=geoType;
          cPtr=std::copy(nodalConn+nodalConnI[i],nodalConn+nodalConnI[i+1],cPtr);
          ciPtr[1]=ciPtr[0]+sz+1;
        }
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::buildUnstructured : Invalid for nodal index for cell #" << i << " !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  ret->setConnectivity(c,cI,true);
  try
  { ret->copyTinyInfoFrom(this); }
  catch(INTERP_KERNEL::Exception&) { }
  return ret.retn();
}

/*!
 * Do nothing for the moment, because there is no policy that allows to split polygons, polyhedrons ... into simplexes
 */
DataArrayIdType *MEDCoupling1DGTUMesh::simplexize(int policy)
{
  mcIdType nbOfCells=getNumberOfCells();
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(nbOfCells,1);
  ret->iota(0);
  return ret.retn();
}

void MEDCoupling1DGTUMesh::reprQuickOverview(std::ostream& stream) const
{
  stream << "MEDCoupling1DGTUMesh C++ instance at " << this << ". Type=";
  if(!_cm)
    {
      stream << "Not defined";
      return ;
    }
  stream << _cm->getRepr() << ". Name : \"" << getName() << "\".";
  stream << " Mesh dimension : " << getMeshDimension() << ".";
  if(!_coords)
    { stream << " No coordinates set !"; return ; }
  if(!_coords->isAllocated())
    { stream << " Coordinates set but not allocated !"; return ; }
  stream << " Space dimension : " << _coords->getNumberOfComponents() << "." << std::endl;
  stream << "Number of nodes : " << _coords->getNumberOfTuples() << ".";
  bool isOK=true;
  try { checkConsistencyLight(); } catch(INTERP_KERNEL::Exception&  /* e */)
  {
      stream << std::endl << "Nodal connectivity NOT set properly !\n";
      isOK=false;
  }
  if(isOK)
    stream << std::endl << "Number of cells : " << getNumberOfCells() << ".";
}

void MEDCoupling1DGTUMesh::shallowCopyConnectivityFrom(const MEDCouplingPointSet *other)
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::shallowCopyConnectivityFrom : input pointer is null !");
  const MEDCoupling1DGTUMesh *otherC=dynamic_cast<const MEDCoupling1DGTUMesh *>(other);
  if(!otherC)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::shallowCopyConnectivityFrom : input pointer is not an MEDCoupling1DGTUMesh instance !");
  setNodalConnectivity(otherC->getNodalConnectivity(),otherC->getNodalConnectivityIndex());
}

MEDCouplingPointSet *MEDCoupling1DGTUMesh::mergeMyselfWithOnSameCoords(const MEDCouplingPointSet *other) const
{
  if(!other)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::mergeMyselfWithOnSameCoords : input other is null !");
  const MEDCoupling1DGTUMesh *otherC=dynamic_cast<const MEDCoupling1DGTUMesh *>(other);
  if(!otherC)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::mergeMyselfWithOnSameCoords : the input other mesh is not of type single statuc geo type unstructured !");
  std::vector<const MEDCoupling1DGTUMesh *> ms(2);
  ms[0]=this;
  ms[1]=otherC;
  return Merge1DGTUMeshesOnSameCoords(ms);
}

MEDCouplingPointSet *MEDCoupling1DGTUMesh::buildPartOfMySelfKeepCoords(const mcIdType *begin, const mcIdType *end) const
{
  checkConsistencyLight();
  MCAuto<MEDCoupling1DGTUMesh> ret(new MEDCoupling1DGTUMesh(getName(),*_cm));
  ret->setCoords(_coords);
  DataArrayIdType *c=0,*ci=0;
  DataArrayIdType::ExtractFromIndexedArrays(begin,end,_conn,_conn_indx,c,ci);
  MCAuto<DataArrayIdType> cSafe(c),ciSafe(ci);
  ret->setNodalConnectivity(c,ci);
  return ret.retn();
}

MEDCouplingPointSet *MEDCoupling1DGTUMesh::buildPartOfMySelfKeepCoordsSlice(mcIdType start, mcIdType end, mcIdType step) const
{
  checkConsistencyLight();
  MCAuto<MEDCoupling1DGTUMesh> ret(new MEDCoupling1DGTUMesh(getName(),*_cm));
  ret->setCoords(_coords);
  DataArrayIdType *c=0,*ci=0;
  DataArrayIdType::ExtractFromIndexedArraysSlice(start,end,step,_conn,_conn_indx,c,ci);
  MCAuto<DataArrayIdType> cSafe(c),ciSafe(ci);
  ret->setNodalConnectivity(c,ci);
  return ret.retn();
}

void MEDCoupling1DGTUMesh::computeNodeIdsAlg(std::vector<bool>& nodeIdsInUse) const
{
  checkConsistency();
  mcIdType sz(ToIdType(nodeIdsInUse.size()));
  for(const mcIdType *conn=_conn->begin();conn!=_conn->end();conn++)
    {
      if(*conn>=0 && *conn<sz)
        nodeIdsInUse[*conn]=true;
      else
        {
          if(*conn!=-1)
            {
              std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::computeNodeIdsAlg : At pos #" << std::distance(_conn->begin(),conn) << " value is " << *conn << " must be in [0," << sz << ") !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
    }
}

void MEDCoupling1DGTUMesh::getReverseNodalConnectivity(DataArrayIdType *revNodal, DataArrayIdType *revNodalIndx) const
{
  checkFullyDefined();
  mcIdType nbOfNodes=getNumberOfNodes();
  mcIdType *revNodalIndxPtr=(mcIdType *)malloc((nbOfNodes+1)*sizeof(mcIdType));
  revNodalIndx->useArray(revNodalIndxPtr,true,DeallocType::C_DEALLOC,nbOfNodes+1,1);
  std::fill(revNodalIndxPtr,revNodalIndxPtr+nbOfNodes+1,0);
  const mcIdType *conn=_conn->begin(),*conni=_conn_indx->begin();
  mcIdType nbOfCells=getNumberOfCells();
  mcIdType nbOfEltsInRevNodal=0;
  for(mcIdType eltId=0;eltId<nbOfCells;eltId++)
    {
      mcIdType nbOfNodesPerCell=conni[eltId+1]-conni[eltId];
      if(nbOfNodesPerCell>=0)
        {
          for(mcIdType j=0;j<nbOfNodesPerCell;j++)
            {
              mcIdType nodeId=conn[conni[eltId]+j];
              if(nodeId==-1) continue;            
              if(nodeId>=0 && nodeId<nbOfNodes)
                {
                  nbOfEltsInRevNodal++;
                  revNodalIndxPtr[nodeId+1]++;
                }
              else
                {
                  std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::getReverseNodalConnectivity : At cell #" << eltId << " presence of nodeId #" << conn[0] << " should be in [0," << nbOfNodes << ") !";
                  throw INTERP_KERNEL::Exception(oss.str().c_str());
                }
            }
        }
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::getReverseNodalConnectivity : At cell #" << eltId << "nodal connectivity is invalid !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  std::transform(revNodalIndxPtr+1,revNodalIndxPtr+nbOfNodes+1,revNodalIndxPtr,revNodalIndxPtr+1,std::plus<mcIdType>());
  conn=_conn->begin();
  mcIdType *revNodalPtr=(mcIdType *)malloc((nbOfEltsInRevNodal)*sizeof(mcIdType));
  revNodal->useArray(revNodalPtr,true,DeallocType::C_DEALLOC,nbOfEltsInRevNodal,1);
  std::fill(revNodalPtr,revNodalPtr+nbOfEltsInRevNodal,-1);
  for(mcIdType eltId=0;eltId<nbOfCells;eltId++)
    {
      mcIdType nbOfNodesPerCell=conni[eltId+1]-conni[eltId];
      for(mcIdType j=0;j<nbOfNodesPerCell;j++)
        {
          mcIdType nodeId=conn[conni[eltId]+j];
          if(nodeId!=-1)
            *std::find_if(revNodalPtr+revNodalIndxPtr[nodeId],revNodalPtr+revNodalIndxPtr[nodeId+1],std::bind(std::equal_to<mcIdType>(),std::placeholders::_1,-1))=eltId;
        }
    }
}

void MEDCoupling1DGTUMesh::checkFullyDefined() const
{
  if(!((const DataArrayIdType *)_conn) || !((const DataArrayIdType *)_conn_indx) || !((const DataArrayDouble *)_coords))
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::checkFullyDefined : part of this is not fully defined.");
}

bool MEDCoupling1DGTUMesh::isEmptyMesh(const std::vector<mcIdType>& tinyInfo) const
{
  throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::isEmptyMesh : not implemented yet !");
}

void MEDCoupling1DGTUMesh::getTinySerializationInformation(std::vector<double>& tinyInfoD, std::vector<mcIdType>& tinyInfo, std::vector<std::string>& littleStrings) const
{
  int it,order;
  double time=getTime(it,order);
  tinyInfo.clear(); tinyInfoD.clear(); littleStrings.clear();
  //
  littleStrings.push_back(getName());
  littleStrings.push_back(getDescription());
  littleStrings.push_back(getTimeUnit());
  //
  std::vector<std::string> littleStrings2,littleStrings3,littleStrings4;
  if((const DataArrayDouble *)_coords)
    _coords->getTinySerializationStrInformation(littleStrings2);
  if((const DataArrayIdType *)_conn)
    _conn->getTinySerializationStrInformation(littleStrings3);
  if((const DataArrayIdType *)_conn_indx)
    _conn_indx->getTinySerializationStrInformation(littleStrings4);
  mcIdType sz0(ToIdType(littleStrings2.size())),sz1(ToIdType(littleStrings3.size())),sz2(ToIdType(littleStrings4.size()));
  littleStrings.insert(littleStrings.end(),littleStrings2.begin(),littleStrings2.end());
  littleStrings.insert(littleStrings.end(),littleStrings3.begin(),littleStrings3.end());
  littleStrings.insert(littleStrings.end(),littleStrings4.begin(),littleStrings4.end());
  //
  tinyInfo.push_back(getCellModelEnum());
  tinyInfo.push_back(it);
  tinyInfo.push_back(order);
  std::vector<mcIdType> tinyInfo2,tinyInfo3,tinyInfo4;
  if((const DataArrayDouble *)_coords)
    _coords->getTinySerializationIntInformation(tinyInfo2);
  if((const DataArrayIdType *)_conn)
    _conn->getTinySerializationIntInformation(tinyInfo3);
  if((const DataArrayIdType *)_conn_indx)
    _conn_indx->getTinySerializationIntInformation(tinyInfo4);
  mcIdType sz3(ToIdType(tinyInfo2.size())),sz4(ToIdType(tinyInfo3.size())),sz5(ToIdType(tinyInfo4.size()));
  tinyInfo.push_back(sz0); tinyInfo.push_back(sz1); tinyInfo.push_back(sz2); tinyInfo.push_back(sz3); tinyInfo.push_back(sz4);  tinyInfo.push_back(sz5);
  tinyInfo.insert(tinyInfo.end(),tinyInfo2.begin(),tinyInfo2.end());
  tinyInfo.insert(tinyInfo.end(),tinyInfo3.begin(),tinyInfo3.end());
  tinyInfo.insert(tinyInfo.end(),tinyInfo4.begin(),tinyInfo4.end());
  //
  tinyInfoD.push_back(time);
}

void MEDCoupling1DGTUMesh::resizeForUnserialization(const std::vector<mcIdType>& tinyInfo, DataArrayIdType *a1, DataArrayDouble *a2, std::vector<std::string>& littleStrings) const
{
  std::vector<mcIdType> tinyInfo2(tinyInfo.begin()+9,tinyInfo.begin()+9+tinyInfo[6]);
  std::vector<mcIdType> tinyInfo1(tinyInfo.begin()+9+tinyInfo[6],tinyInfo.begin()+9+tinyInfo[6]+tinyInfo[7]);
  std::vector<mcIdType> tinyInfo12(tinyInfo.begin()+9+tinyInfo[6]+tinyInfo[7],tinyInfo.begin()+9+tinyInfo[6]+tinyInfo[7]+tinyInfo[8]);
  MCAuto<DataArrayIdType> p1(DataArrayIdType::New()); p1->resizeForUnserialization(tinyInfo1);
  MCAuto<DataArrayIdType> p2(DataArrayIdType::New()); p2->resizeForUnserialization(tinyInfo12);
  std::vector<const DataArrayIdType *> v(2); v[0]=p1; v[1]=p2;
  p2=DataArrayIdType::Aggregate(v);
  a2->resizeForUnserialization(tinyInfo2);
  a1->alloc(p2->getNbOfElems(),1);
}

void MEDCoupling1DGTUMesh::serialize(DataArrayIdType *&a1, DataArrayDouble *&a2) const
{
  mcIdType sz(0);
  if((const DataArrayIdType *)_conn)
    if(_conn->isAllocated())
      sz=_conn->getNbOfElems();
  if((const DataArrayIdType *)_conn_indx)
    if(_conn_indx->isAllocated())
      sz+=_conn_indx->getNbOfElems();
  a1=DataArrayIdType::New();
  a1->alloc(sz,1);
  mcIdType *work(a1->getPointer());
  if(sz!=0 && (const DataArrayIdType *)_conn)
    work=std::copy(_conn->begin(),_conn->end(),a1->getPointer());
  if(sz!=0 && (const DataArrayIdType *)_conn_indx)
    std::copy(_conn_indx->begin(),_conn_indx->end(),work);
  sz=0;
  if((const DataArrayDouble *)_coords)
    if(_coords->isAllocated())
      sz=_coords->getNbOfElems();
  a2=DataArrayDouble::New();
  a2->alloc(sz,1);
  if(sz!=0 && (const DataArrayDouble *)_coords)
    std::copy(_coords->begin(),_coords->end(),a2->getPointer());
}

void MEDCoupling1DGTUMesh::unserialization(const std::vector<double>& tinyInfoD, const std::vector<mcIdType>& tinyInfo, const DataArrayIdType *a1, DataArrayDouble *a2,
                                           const std::vector<std::string>& littleStrings)
{
  INTERP_KERNEL::NormalizedCellType gt((INTERP_KERNEL::NormalizedCellType)tinyInfo[0]);
  _cm=&INTERP_KERNEL::CellModel::GetCellModel(gt);
  setName(littleStrings[0]);
  setDescription(littleStrings[1]);
  setTimeUnit(littleStrings[2]);
  setTime(tinyInfoD[0],FromIdType<int>(tinyInfo[1]),FromIdType<int>(tinyInfo[2]));
  mcIdType sz0(tinyInfo[3]),sz1(tinyInfo[4]),sz2(tinyInfo[5]),sz3(tinyInfo[6]),sz4(tinyInfo[7]),sz5(tinyInfo[8]);
  //
  _coords=DataArrayDouble::New();
  std::vector<mcIdType> tinyInfo2(tinyInfo.begin()+9,tinyInfo.begin()+9+sz3);
  _coords->resizeForUnserialization(tinyInfo2);
  std::copy(a2->begin(),a2->end(),_coords->getPointer());
  _conn=DataArrayIdType::New();
  std::vector<mcIdType> tinyInfo3(tinyInfo.begin()+9+sz3,tinyInfo.begin()+9+sz3+sz4);
  _conn->resizeForUnserialization(tinyInfo3);
  std::copy(a1->begin(),a1->begin()+_conn->getNbOfElems(),_conn->getPointer());
  _conn_indx=DataArrayIdType::New();
  std::vector<mcIdType> tinyInfo4(tinyInfo.begin()+9+sz3+sz4,tinyInfo.begin()+9+sz3+sz4+sz5);
  _conn_indx->resizeForUnserialization(tinyInfo4);
  std::copy(a1->begin()+_conn->getNbOfElems(),a1->end(),_conn_indx->getPointer());
  std::vector<std::string> littleStrings2(littleStrings.begin()+3,littleStrings.begin()+3+sz0);
  _coords->finishUnserialization(tinyInfo2,littleStrings2);
  std::vector<std::string> littleStrings3(littleStrings.begin()+3+sz0,littleStrings.begin()+3+sz0+sz1);
  _conn->finishUnserialization(tinyInfo3,littleStrings3);
  std::vector<std::string> littleStrings4(littleStrings.begin()+3+sz0+sz1,littleStrings.begin()+3+sz0+sz1+sz2);
  _conn_indx->finishUnserialization(tinyInfo4,littleStrings4);
}

/*!
 * Finds nodes not used in any cell and returns an array giving a new id to every node
 * by excluding the unused nodes, for which the array holds -1. The result array is
 * a mapping in "Old to New" mode.
 *  \return DataArrayIdType * - a new instance of DataArrayIdType. Its length is \a
 *          this->getNumberOfNodes(). It holds for each node of \a this mesh either -1
 *          if the node is unused or a new id else. The caller is to delete this
 *          array using decrRef() as it is no more needed.
 *  \throw If the coordinates array is not set.
 *  \throw If the nodal connectivity of cells is not defined.
 *  \throw If the nodal connectivity includes an invalid id.
 *  \sa MEDCoupling1DGTUMesh::getNodeIdsInUse, areAllNodesFetched
 */
DataArrayIdType *MEDCoupling1DGTUMesh::computeFetchedNodeIds() const
{
  checkConsistency();
  mcIdType nbNodes(getNumberOfNodes());
  std::vector<bool> fetchedNodes(nbNodes,false);
  computeNodeIdsAlg(fetchedNodes);
  mcIdType sz(ToIdType(std::count(fetchedNodes.begin(),fetchedNodes.end(),true)));
  MCAuto<DataArrayIdType> ret(DataArrayIdType::New()); ret->alloc(sz,1);
  mcIdType *retPtr(ret->getPointer());
  for(mcIdType i=0;i<nbNodes;i++)
    if(fetchedNodes[i])
      *retPtr++=i;
  return ret.retn();
}

/*!
 * Finds nodes not used in any cell and returns an array giving a new id to every node
 * by excluding the unused nodes, for which the array holds -1. The result array is
 * a mapping in "Old to New" mode. 
 *  \param [out] nbrOfNodesInUse - number of node ids present in the nodal connectivity.
 *  \return DataArrayIdType * - a new instance of DataArrayIdType. Its length is \a
 *          this->getNumberOfNodes(). It holds for each node of \a this mesh either -1
 *          if the node is unused or a new id else. The caller is to delete this
 *          array using decrRef() as it is no more needed.  
 *  \throw If the coordinates array is not set.
 *  \throw If the nodal connectivity of cells is not defined.
 *  \throw If the nodal connectivity includes an invalid id.
 *  \sa MEDCoupling1DGTUMesh::computeFetchedNodeIds, areAllNodesFetched
 */
DataArrayIdType *MEDCoupling1DGTUMesh::getNodeIdsInUse(mcIdType& nbrOfNodesInUse) const
{
  nbrOfNodesInUse=-1;
  mcIdType nbOfNodes=getNumberOfNodes();
  mcIdType nbOfCells=getNumberOfCells();//checkConsistencyLight
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New();
  ret->alloc(nbOfNodes,1);
  mcIdType *traducer=ret->getPointer();
  std::fill(traducer,traducer+nbOfNodes,-1);
  const mcIdType *conn=_conn->begin(),*conni(_conn_indx->begin());
  for(mcIdType i=0;i<nbOfCells;i++,conni++)
    {
      mcIdType nbNodesPerCell=conni[1]-conni[0];
      for(mcIdType j=0;j<nbNodesPerCell;j++)
        {
          mcIdType nodeId=conn[conni[0]+j];
          if(nodeId==-1) continue;
          if(nodeId>=0 && nodeId<nbOfNodes)
            traducer[nodeId]=1;
          else
            {
              std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::getNodeIdsInUse : In cell #" << i  << " presence of node id " <<  nodeId << " not in [0," << nbOfNodes << ") !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
    }
  nbrOfNodesInUse=ToIdType(std::count(traducer,traducer+nbOfNodes,1));
  std::transform(traducer,traducer+nbOfNodes,traducer,MEDCouplingAccVisit());
  return ret.retn();
}

/*!
 * This method renumbers only nodal connectivity in \a this. The renumbering is only an offset applied. So this method is a specialization of
 * \a renumberNodesInConn. \b WARNING, this method does not check that the resulting node ids in the nodal connectivity is in a valid range !
 *
 * \param [in] offset - specifies the offset to be applied on each element of connectivity.
 *
 * \sa renumberNodesInConn
 */
void MEDCoupling1DGTUMesh::renumberNodesWithOffsetInConn(mcIdType offset)
{
  getNumberOfCells();//only to check that all is well defined.
  //
  mcIdType nbOfTuples(_conn->getNumberOfTuples());
  mcIdType *pt(_conn->getPointer());
  for(mcIdType i=0;i<nbOfTuples;i++,pt++)
    {
      if(*pt==-1) continue;
      *pt+=offset;
    }
  //
  updateTime();
}

/*!
 *  Same than renumberNodesInConn(const mcIdType *) except that here the format of old-to-new traducer is using map instead
 *  of array. This method is dedicated for renumbering from a big set of nodes the a tiny set of nodes which is the case during extraction
 *  of a big mesh.
 */
void MEDCoupling1DGTUMesh::renumberNodesInConn(const INTERP_KERNEL::HashMap<mcIdType,mcIdType>& newNodeNumbersO2N)
{
  this->renumberNodesInConnT< INTERP_KERNEL::HashMap<mcIdType,mcIdType> >(newNodeNumbersO2N);
}

/*!
 *  Same than renumberNodesInConn(const mcIdType *) except that here the format of old-to-new traducer is using map instead
 *  of array. This method is dedicated for renumbering from a big set of nodes the a tiny set of nodes which is the case during extraction
 *  of a big mesh.
 */
void MEDCoupling1DGTUMesh::renumberNodesInConn(const std::map<mcIdType,mcIdType>& newNodeNumbersO2N)
{
  this->renumberNodesInConnT< std::map<mcIdType,mcIdType> >(newNodeNumbersO2N);
}

/*!
 * Changes ids of nodes within the nodal connectivity arrays according to a permutation
 * array in "Old to New" mode. The node coordinates array is \b not changed by this method.
 * This method is a generalization of shiftNodeNumbersInConn().
 *  \warning This method performs no check of validity of new ids. **Use it with care !**
 *  \param [in] newNodeNumbersO2N - a permutation array, of length \a
 *         this->getNumberOfNodes(), in "Old to New" mode. 
 *         See \ref numbering for more info on renumbering modes.
 *  \throw If the nodal connectivity of cells is not defined.
 */
void MEDCoupling1DGTUMesh::renumberNodesInConn(const mcIdType *newNodeNumbersO2N)
{
  getNumberOfCells();//only to check that all is well defined.
  //
  mcIdType nbElemsIn(getNumberOfNodes()),nbOfTuples(_conn->getNumberOfTuples());
  mcIdType *pt(_conn->getPointer());
  for(mcIdType i=0;i<nbOfTuples;i++,pt++)
    {
      if(*pt==-1) continue;
      if(*pt>=0 && *pt<nbElemsIn)
        *pt=newNodeNumbersO2N[*pt];
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::renumberNodesInConn : error on tuple #" << i << " value is " << *pt << " and indirectionnal array as a size equal to " << nbElemsIn;
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  //
  updateTime();
}

/*!
 * Keeps from \a this only cells which constituing point id are in the ids specified by [\a begin,\a end).
 * The resulting cell ids are stored at the end of the 'cellIdsKept' parameter.
 * Parameter \a fullyIn specifies if a cell that has part of its nodes in ids array is kept or not.
 * If \a fullyIn is true only cells whose ids are \b fully contained in [\a begin,\a end) tab will be kept.
 *
 * \param [in] begin input start of array of node ids.
 * \param [in] end input end of array of node ids.
 * \param [in] fullyIn input that specifies if all node ids must be in [\a begin,\a end) array to consider cell to be in.
 * \param [in,out] cellIdsKeptArr array where all candidate cell ids are put at the end.
 */
void MEDCoupling1DGTUMesh::fillCellIdsToKeepFromNodeIds(const mcIdType *begin, const mcIdType *end, bool fullyIn, DataArrayIdType *&cellIdsKeptArr) const
{
  mcIdType nbOfCells=getNumberOfCells();
  MCAuto<DataArrayIdType> cellIdsKept=DataArrayIdType::New(); cellIdsKept->alloc(0,1);
  mcIdType tmp=-1;
  mcIdType sz=_conn->getMaxValue(tmp); sz=std::max(sz,ToIdType(0))+1;
  std::vector<bool> fastFinder(sz,false);
  for(const mcIdType *work=begin;work!=end;work++)
    if(*work>=0 && *work<sz)
      fastFinder[*work]=true;
  const mcIdType *conn=_conn->begin(),*conni=_conn_indx->begin();
  for(mcIdType i=0;i<nbOfCells;i++,conni++)
    {
      int ref=0,nbOfHit=0;
      mcIdType nbNodesPerCell=conni[1]-conni[0];
      if(nbNodesPerCell>=0)
        {
          for(mcIdType j=0;j<nbNodesPerCell;j++)
            {
              mcIdType nodeId=conn[conni[0]+j];
              if(nodeId>=0)
                {
                  ref++;
                  if(fastFinder[nodeId])
                    nbOfHit++;
                }
            }
        }
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::fillCellIdsToKeepFromNodeIds : invalid index array for cell #" << i << " !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
      if((ref==nbOfHit && fullyIn) || (nbOfHit!=0 && !fullyIn))
        cellIdsKept->pushBackSilent(i);
    }
  cellIdsKeptArr=cellIdsKept.retn();
}

void MEDCoupling1DGTUMesh::allocateCells(mcIdType nbOfCells)
{
  if(nbOfCells<0)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::allocateCells : the input number of cells should be >= 0 !");
  _conn=DataArrayIdType::New();
  _conn->reserve(nbOfCells*3);
  _conn_indx=DataArrayIdType::New();
  _conn_indx->reserve(nbOfCells+1); _conn_indx->pushBackSilent(0);
  declareAsNew();
}

/*!
 * Appends at the end of \a this a cell having nodal connectivity array defined in [ \a nodalConnOfCellBg, \a nodalConnOfCellEnd ).
 *
 * \param [in] nodalConnOfCellBg - the begin (included) of nodal connectivity of the cell to add.
 * \param [in] nodalConnOfCellEnd - the end (excluded) of nodal connectivity of the cell to add.
 * \throw If the length of the input nodal connectivity array of the cell to add is not equal to number of nodes per cell relative to the unique geometric type
 *        attached to \a this.
 * \throw If the nodal connectivity array in \a this is null (call MEDCoupling1SGTUMesh::allocateCells before).
 */
void MEDCoupling1DGTUMesh::insertNextCell(const mcIdType *nodalConnOfCellBg, const mcIdType *nodalConnOfCellEnd)
{
  std::size_t sz(std::distance(nodalConnOfCellBg,nodalConnOfCellEnd));
  DataArrayIdType *c(_conn),*c2(_conn_indx);
  if(c && c2)
    {
      mcIdType pos=c2->back();
      if(pos==c->getNumberOfTuples())
        {
          c->pushBackValsSilent(nodalConnOfCellBg,nodalConnOfCellEnd);
          c2->pushBackSilent(pos+ToIdType(sz));
        }
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::insertNextCell : The nodal index array (end=" << pos << ") mismatches with nodal array (length=" << c->getNumberOfTuples() << ") !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  else
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::insertNextCell : nodal connectivity array is null ! Call MEDCoupling1DGTUMesh::allocateCells before !");
}

void MEDCoupling1DGTUMesh::setNodalConnectivity(DataArrayIdType *nodalConn, DataArrayIdType *nodalConnIndex)
{
  if(nodalConn)
    nodalConn->incrRef();
  _conn=nodalConn;
  if(nodalConnIndex)
    nodalConnIndex->incrRef();
  _conn_indx=nodalConnIndex;
  declareAsNew();
}

/*!
 * \return DataArrayIdType * - the internal reference to the nodal connectivity. The caller is not responsible to deallocate it.
 */
DataArrayIdType *MEDCoupling1DGTUMesh::getNodalConnectivity() const
{
  const DataArrayIdType *ret(_conn);
  return const_cast<DataArrayIdType *>(ret);
}

/*!
 * \return DataArrayIdType * - the internal reference to the nodal connectivity index. The caller is not responsible to deallocate it.
 */
DataArrayIdType *MEDCoupling1DGTUMesh::getNodalConnectivityIndex() const
{
  const DataArrayIdType *ret(_conn_indx);
  return const_cast<DataArrayIdType *>(ret);
}

/*!
 * See the definition of the nodal connectivity pack \ref MEDCoupling1DGTUMesh::isPacked "here".
 * This method tries to build a new instance geometrically equivalent to \a this, by limiting at most the number of new object (nodal connectivity).
 * Geometrically the returned mesh is equal to \a this. So if \a this is already packed, the return value is a shallow copy of \a this.
 *
 * Whatever the status of pack of \a this, the coordinates array of the returned newly created instance is the same than those in \a this.
 * 
 * \param [out] isShallowCpyOfNodalConnn - tells if the returned instance share the same pair of nodal connectivity arrays (true) or if nodal
 *              connectivity arrays are different (false)
 * \return a new object to be managed by the caller.
 * 
 * \sa MEDCoupling1DGTUMesh::retrievePackedNodalConnectivity, MEDCoupling1DGTUMesh::isPacked
 */
MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::copyWithNodalConnectivityPacked(bool& isShallowCpyOfNodalConnn) const
{
  MCAuto<MEDCoupling1DGTUMesh> ret(new MEDCoupling1DGTUMesh(getName(),*_cm));
  DataArrayIdType *nc=0,*nci=0;
  isShallowCpyOfNodalConnn=retrievePackedNodalConnectivity(nc,nci);
  MCAuto<DataArrayIdType> ncs(nc),ncis(nci);
  ret->_conn=ncs; ret->_conn_indx=ncis;
  ret->setCoords(getCoords());
  return ret.retn();
}

/*!
 * This method allows to compute, if needed, the packed nodal connectivity pair.
 * Indeed, it is possible to store in \a this a nodal connectivity array bigger than ranges covered by nodal connectivity index array.
 * It is typically the case when nodalConnIndx starts with an id greater than 0, and finishes with id less than number of tuples in \c this->_conn.
 * 
 * If \a this looks packed (the front of nodal connectivity index equal to 0 and back of connectivity index equal to number of tuple of nodal connectivity array)
 * true will be returned and respectively \a this->_conn and \a this->_conn_indx (with ref counter incremented). This is the classical case.
 *
 * If nodal connectivity index points to a subpart of nodal connectivity index the packed pair of arrays will be computed (new objects) and returned and false
 * will be returned.
 * 
 * This method return 3 elements.
 * \param [out] nodalConn - a pointer that can be equal to \a this->_conn if true is returned (general case). Whatever the value of return parameter
 *                          this pointer can be seen as a new object, that is to managed by the caller.
 * \param [out] nodalConnIndx - a pointer that can be equal to \a this->_conn_indx if true is returned (general case). Whatever the value of return parameter
 *                              this pointer can be seen as a new object, that is to managed by the caller.
 * \return bool - an indication of the content of the 2 output parameters. If true, \a this looks packed (general case), if true, \a this is not packed then
 * output parameters are newly created objects.
 *
 * \throw if \a this does not pass MEDCoupling1DGTUMesh::checkConsistencyLight test
 */
bool MEDCoupling1DGTUMesh::retrievePackedNodalConnectivity(DataArrayIdType *&nodalConn, DataArrayIdType *&nodalConnIndx) const
{
  if(isPacked())//performs the checkConsistencyLight
    {
      const DataArrayIdType *c0(_conn),*c1(_conn_indx);
      nodalConn=const_cast<DataArrayIdType *>(c0); nodalConnIndx=const_cast<DataArrayIdType *>(c1);
      nodalConn->incrRef(); nodalConnIndx->incrRef();
      return true;
    }
  mcIdType bg=_conn_indx->front(),end=_conn_indx->back();
  MCAuto<DataArrayIdType> nc(_conn->selectByTupleIdSafeSlice(bg,end,1));
  MCAuto<DataArrayIdType> nci(_conn_indx->deepCopy());
  nci->applyLin(1,-bg);
  nodalConn=nc.retn(); nodalConnIndx=nci.retn();
  return false;
}

/*
 * If \a this looks packed (the front of nodal connectivity index equal to 0 and back of connectivity index equal to number of tuple of nodal connectivity array)
 * true will be returned and respectively \a this->_conn and \a this->_conn_indx (with ref counter incremented). This is the classical case.
 * If nodal connectivity index points to a subpart of nodal connectivity index false will be returned.
 * \return bool - true if \a this looks packed, false is not.
 *
 * \throw if \a this does not pass MEDCoupling1DGTUMesh::checkConsistencyLight test
 */
bool MEDCoupling1DGTUMesh::isPacked() const
{
  checkConsistencyLight();
  return _conn_indx->front()==0 && _conn_indx->back()==_conn->getNumberOfTuples();
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::Merge1DGTUMeshes(const MEDCoupling1DGTUMesh *mesh1, const MEDCoupling1DGTUMesh *mesh2)
{
  std::vector<const MEDCoupling1DGTUMesh *> tmp(2);
  tmp[0]=const_cast<MEDCoupling1DGTUMesh *>(mesh1); tmp[1]=const_cast<MEDCoupling1DGTUMesh *>(mesh2);
  return Merge1DGTUMeshes(tmp);
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::Merge1DGTUMeshes(std::vector<const MEDCoupling1DGTUMesh *>& a)
{
  std::size_t sz=a.size();
  if(sz==0)
    return Merge1DGTUMeshesLL(a);
  for(std::size_t ii=0;ii<sz;ii++)
    if(!a[ii])
      {
        std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::Merge1DGTUMeshes : item #" << ii << " in input array of size "<< sz << " is empty !";
        throw INTERP_KERNEL::Exception(oss.str().c_str());
      }
  const INTERP_KERNEL::CellModel *cm=&(a[0]->getCellModel());
  for(std::size_t ii=0;ii<sz;ii++)
    if(&(a[ii]->getCellModel())!=cm)
      throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshes : all items must have the same geo type !");
  std::vector< MCAuto<MEDCoupling1DGTUMesh> > bb(sz);
  std::vector< const MEDCoupling1DGTUMesh * > aa(sz);
  std::size_t spaceDimUndef=-3, spaceDim=spaceDimUndef;
  for(std::size_t i=0;i<sz && spaceDim==spaceDimUndef;i++)
    {
      const MEDCoupling1DGTUMesh *cur=a[i];
      const DataArrayDouble *coo=cur->getCoords();
      if(coo)
        spaceDim=coo->getNumberOfComponents();
    }
  if(spaceDim==spaceDimUndef)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshes : no spaceDim specified ! unable to perform merge !");
  for(std::size_t i=0;i<sz;i++)
    {
      bb[i]=a[i]->buildSetInstanceFromThis(spaceDim);
      aa[i]=bb[i];
    }
  return Merge1DGTUMeshesLL(aa);
}

/*!
 * \throw If presence of a null instance in the input vector \a a.
 * \throw If a is empty
 */
MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::Merge1DGTUMeshesOnSameCoords(std::vector<const MEDCoupling1DGTUMesh *>& a)
{
  if(a.empty())
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshesOnSameCoords : input array must be NON EMPTY !");
  std::vector<const MEDCoupling1DGTUMesh *>::const_iterator it=a.begin();
  if(!(*it))
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshesOnSameCoords : null instance in the first element of input vector !");
  std::vector< MCAuto<MEDCoupling1DGTUMesh> > objs(a.size());
  std::vector<const DataArrayIdType *> ncs(a.size()),ncis(a.size());
  (*it)->getNumberOfCells();//to check that all is OK
  const DataArrayDouble *coords=(*it)->getCoords();
  const INTERP_KERNEL::CellModel *cm=&((*it)->getCellModel());
  bool tmp;
  objs[0]=(*it)->copyWithNodalConnectivityPacked(tmp);
  ncs[0]=objs[0]->getNodalConnectivity(); ncis[0]=objs[0]->getNodalConnectivityIndex();
  it++;
  for(int i=1;it!=a.end();i++,it++)
    {
      if(!(*it))
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshesOnSameCoords : presence of null instance !");
      if(cm!=&((*it)->getCellModel()))
        throw INTERP_KERNEL::Exception("Geometric types mismatches, Merge1DGTUMeshes impossible !");
      (*it)->getNumberOfCells();//to check that all is OK
      objs[i]=(*it)->copyWithNodalConnectivityPacked(tmp);
      ncs[i]=objs[i]->getNodalConnectivity(); ncis[i]=objs[i]->getNodalConnectivityIndex();
      if(coords!=(*it)->getCoords())
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshesOnSameCoords : not lying on same coords !");
    }
  MCAuto<MEDCoupling1DGTUMesh> ret(new MEDCoupling1DGTUMesh("merge",*cm));
  ret->setCoords(coords);
  ret->_conn=DataArrayIdType::Aggregate(ncs);
  ret->_conn_indx=DataArrayIdType::AggregateIndexes(ncis);
  return ret.retn();
}

/*!
 * Assume that all instances in \a a are non null. If null it leads to a crash. That's why this method is assigned to be low level (LL)
 */
MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::Merge1DGTUMeshesLL(std::vector<const MEDCoupling1DGTUMesh *>& a)
{
  if(a.empty())
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::Merge1DGTUMeshes : input array must be NON EMPTY !");
  std::vector< MCAuto<MEDCoupling1DGTUMesh> > objs(a.size());
  std::vector<const DataArrayIdType *> ncs(a.size()),ncis(a.size());
  std::vector<const MEDCoupling1DGTUMesh *>::const_iterator it=a.begin();
  std::vector<mcIdType> nbNodesPerElt(a.size());
  std::size_t nbOfCells=(*it)->getNumberOfCells();
  bool tmp;
  objs[0]=(*it)->copyWithNodalConnectivityPacked(tmp);
  ncs[0]=objs[0]->getNodalConnectivity(); ncis[0]=objs[0]->getNodalConnectivityIndex();
  nbNodesPerElt[0]=0;
  mcIdType prevNbOfNodes=(*it)->getNumberOfNodes();
  const INTERP_KERNEL::CellModel *cm=&((*it)->getCellModel());
  it++;
  for(int i=1;it!=a.end();i++,it++)
    {
      if(cm!=&((*it)->getCellModel()))
        throw INTERP_KERNEL::Exception("Geometric types mismatches, Merge1DGTUMeshes impossible !");
      objs[i]=(*it)->copyWithNodalConnectivityPacked(tmp);
      ncs[i]=objs[i]->getNodalConnectivity(); ncis[i]=objs[i]->getNodalConnectivityIndex();
      nbOfCells+=(*it)->getNumberOfCells();
      nbNodesPerElt[i]=nbNodesPerElt[i-1]+prevNbOfNodes;
      prevNbOfNodes=(*it)->getNumberOfNodes();
    }
  std::vector<const MEDCouplingPointSet *> aps(a.size());
  std::copy(a.begin(),a.end(),aps.begin());
  MCAuto<DataArrayDouble> pts=MergeNodesArray(aps);
  MCAuto<MEDCoupling1DGTUMesh> ret(new MEDCoupling1DGTUMesh("merge",*cm));
  ret->setCoords(pts);
  ret->_conn=AggregateNodalConnAndShiftNodeIds(ncs,nbNodesPerElt);
  ret->_conn_indx=DataArrayIdType::AggregateIndexes(ncis);
  return ret.retn();
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::buildSetInstanceFromThis(std::size_t spaceDim) const
{
  MCAuto<MEDCoupling1DGTUMesh> ret(new MEDCoupling1DGTUMesh(getName(),*_cm));
  MCAuto<DataArrayIdType> tmp1,tmp2;
  const DataArrayIdType *nodalConn(_conn),*nodalConnI(_conn_indx);
  if(!nodalConn)
    {
      tmp1=DataArrayIdType::New(); tmp1->alloc(0,1);
    }
  else
    tmp1=_conn;
  ret->_conn=tmp1;
  //
  if(!nodalConnI)
    {
      tmp2=DataArrayIdType::New(); tmp2->alloc(1,1); tmp2->setIJ(0,0,0);
    }
  else
    tmp2=_conn_indx;
  ret->_conn_indx=tmp2;
  //
  if(!_coords)
    {
      MCAuto<DataArrayDouble> coords=DataArrayDouble::New(); coords->alloc(0,spaceDim);
      ret->setCoords(coords);
    }
  else
    ret->setCoords(_coords);
  return ret.retn();
}

/*!
 * This method aggregate the bbox of each cell and put it into bbox parameter.
 * 
 * \param [in] arcDetEps - a parameter specifying in case of 2D quadratic polygon cell the detection limit between linear and arc circle. (By default 1e-12)
 *                         For all other cases this input parameter is ignored.
 * \return DataArrayDouble * - newly created object (to be managed by the caller) \a this number of cells tuples and 2*spacedim components.
 * 
 * \throw If \a this is not fully set (coordinates and connectivity).
 * \throw If a cell in \a this has no valid nodeId.
 */
DataArrayDouble *MEDCoupling1DGTUMesh::getBoundingBoxForBBTree(double arcDetEps) const
{
  checkFullyDefined();
  mcIdType spaceDim(getSpaceDimension()),nbOfCells(getNumberOfCells()),nbOfNodes(getNumberOfNodes());
  MCAuto<DataArrayDouble> ret(DataArrayDouble::New()); ret->alloc(nbOfCells,2*spaceDim);
  double *bbox(ret->getPointer());
  for(mcIdType i=0;i<nbOfCells*spaceDim;i++)
    {
      bbox[2*i]=std::numeric_limits<double>::max();
      bbox[2*i+1]=-std::numeric_limits<double>::max();
    }
  const double *coordsPtr(_coords->getConstPointer());
  const mcIdType *conn(_conn->getConstPointer()),*connI(_conn_indx->getConstPointer());
  for(mcIdType i=0;i<nbOfCells;i++)
    {
      mcIdType offset=connI[i];
      mcIdType nbOfNodesForCell(connI[i+1]-offset),kk(0);
      for(mcIdType j=0;j<nbOfNodesForCell;j++)
        {
          mcIdType nodeId=conn[offset+j];
          if(nodeId>=0 && nodeId<nbOfNodes)
            {
              for(int k=0;k<spaceDim;k++)
                {
                  bbox[2*spaceDim*i+2*k]=std::min(bbox[2*spaceDim*i+2*k],coordsPtr[spaceDim*nodeId+k]);
                  bbox[2*spaceDim*i+2*k+1]=std::max(bbox[2*spaceDim*i+2*k+1],coordsPtr[spaceDim*nodeId+k]);
                }
              kk++;
            }
        }
      if(kk==0)
        {
          std::ostringstream oss; oss << "MEDCoupling1SGTUMesh::getBoundingBoxForBBTree : cell #" << i << " contains no valid nodeId !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  return ret.retn();
}

/*!
 * Returns the cell field giving for each cell in \a this its diameter. Diameter means the max length of all possible SEG2 in the cell.
 *
 * \return a new instance of field containing the result. The returned instance has to be deallocated by the caller.
 */
MEDCouplingFieldDouble *MEDCoupling1DGTUMesh::computeDiameterField() const
{
  throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::computeDiameterField : not implemented yet for dynamic types !");
}

std::vector<mcIdType> MEDCoupling1DGTUMesh::BuildAPolygonFromParts(const std::vector< std::vector<mcIdType> >& parts)
{
  std::vector<mcIdType> ret;
  if(parts.empty())
    return ret;
  ret.insert(ret.end(),parts[0].begin(),parts[0].end());
  mcIdType ref(ret.back());
  std::size_t sz(parts.size()),nbh(1);
  std::vector<bool> b(sz,true); b[0]=false;
  while(nbh<sz)
    {
      std::size_t i(0);
      for(;i<sz;i++) if(b[i] && parts[i].front()==ref) { ret.insert(ret.end(),parts[i].begin()+1,parts[i].end()); nbh++; break; }
      if(i<sz)
        ref=ret.back();
      else
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::BuildAPolygonFromParts : the input vector is not a part of a single polygon !");
    }
  if(ret.back()==ret.front())
    ret.pop_back();
  return ret;
}

/*!
 * This method invert orientation of all cells in \a this. 
 * After calling this method the absolute value of measure of cells in \a this are the same than before calling.
 * This method only operates on the connectivity so coordinates are not touched at all.
 */
void MEDCoupling1DGTUMesh::invertOrientationOfAllCells()
{
  checkConsistencyOfConnectivity();
  INTERP_KERNEL::AutoCppPtr<INTERP_KERNEL::OrientationInverter> oi(INTERP_KERNEL::OrientationInverter::BuildInstanceFrom(getCellModelEnum()));
  mcIdType nbCells=getNumberOfCells();
  const mcIdType *connI(_conn_indx->begin());
  mcIdType *conn(_conn->getPointer());
  for(mcIdType i=0;i<nbCells;i++)
    oi->operate(conn+connI[i],conn+connI[i+1]);
  updateTime();
}

/*!
 * This method performs an aggregation of \a nodalConns (as DataArrayIdType::Aggregate does) but in addition of that a shift is applied on the 
 * values contained in \a nodalConns using corresponding offset specified in input \a offsetInNodeIdsPerElt.
 * But it also manage the values -1, that have a semantic in MEDCoupling1DGTUMesh class (separator for polyhedron).
 *
 * \param [in] nodalConns - a list of nodal connectivity arrays same size than \a offsetInNodeIdsPerElt.
 * \param [in] offsetInNodeIdsPerElt - a list of offsets to apply.
 * \return DataArrayIdType * - A new object (to be managed by the caller) that is the result of the aggregation.
 * \throw If \a nodalConns or \a offsetInNodeIdsPerElt are empty.
 * \throw If \a nodalConns and \a offsetInNodeIdsPerElt have not the same size.
 * \throw If presence of null pointer in \a nodalConns.
 * \throw If presence of not allocated or array with not exactly one component in \a nodalConns.
 */
DataArrayIdType *MEDCoupling1DGTUMesh::AggregateNodalConnAndShiftNodeIds(const std::vector<const DataArrayIdType *>& nodalConns, const std::vector<mcIdType>& offsetInNodeIdsPerElt)
{
  std::size_t sz1(nodalConns.size()),sz2(offsetInNodeIdsPerElt.size());
  if(sz1!=sz2)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::AggregateNodalConnAndShiftNodeIds : input vectors do not have the same size !");
  if(sz1==0)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::AggregateNodalConnAndShiftNodeIds : empty vectors in input !");
  mcIdType nbOfTuples=0;
  for(std::vector<const DataArrayIdType *>::const_iterator it=nodalConns.begin();it!=nodalConns.end();it++)
    {
      if(!(*it))
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::AggregateNodalConnAndShiftNodeIds : presence of null pointer in input vector !");
      if(!(*it)->isAllocated())
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::AggregateNodalConnAndShiftNodeIds : presence of non allocated array in input vector !");
      if((*it)->getNumberOfComponents()!=1)
        throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::AggregateNodalConnAndShiftNodeIds : presence of array with not exactly one component !");
      nbOfTuples+=(*it)->getNumberOfTuples();
    }
  MCAuto<DataArrayIdType> ret=DataArrayIdType::New(); ret->alloc(nbOfTuples,1);
  mcIdType *pt=ret->getPointer();
  mcIdType i=0;
  for(std::vector<const DataArrayIdType *>::const_iterator it=nodalConns.begin();it!=nodalConns.end();it++,i++)
    {
      mcIdType curNbt=(*it)->getNumberOfTuples();
      const mcIdType *inPt=(*it)->begin();
      mcIdType offset=offsetInNodeIdsPerElt[i];
      for(mcIdType j=0;j<curNbt;j++,pt++)
        {
          if(inPt[j]!=-1)
            *pt=inPt[j]+offset;
          else
            *pt=-1;
        }
    }
  return ret.retn();
}

MEDCoupling1DGTUMesh *MEDCoupling1DGTUMesh::New(const MEDCouplingUMesh *m)
{
  if(!m)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::New : input mesh is null !");
  std::set<INTERP_KERNEL::NormalizedCellType> gts(m->getAllGeoTypes());
  if(gts.size()!=1)
    throw INTERP_KERNEL::Exception("MEDCoupling1DGTUMesh::New : input mesh must have exactly one geometric type !");
  mcIdType geoType(ToIdType(*gts.begin()));
  MCAuto<MEDCoupling1DGTUMesh> ret(MEDCoupling1DGTUMesh::New(m->getName(),*gts.begin()));
  ret->setCoords(m->getCoords()); ret->setDescription(m->getDescription());
  mcIdType nbCells=m->getNumberOfCells();
  MCAuto<DataArrayIdType> conn(DataArrayIdType::New()),connI(DataArrayIdType::New());
  conn->alloc(m->getNodalConnectivityArrayLen()-nbCells,1); connI->alloc(nbCells+1,1);
  mcIdType *c(conn->getPointer()),*ci(connI->getPointer()); *ci=0;
  const mcIdType *cin(m->getNodalConnectivity()->begin()),*ciin(m->getNodalConnectivityIndex()->begin());
  for(mcIdType i=0;i<nbCells;i++,ciin++,ci++)
    {
      if(cin[ciin[0]]==geoType)
        {
          if(ciin[1]-ciin[0]>=1)
            {
              c=std::copy(cin+ciin[0]+1,cin+ciin[1],c);
              ci[1]=ci[0]+ciin[1]-ciin[0]-1;
            }
          else
            {
              std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::New(const MEDCouplingUMesh *m) : something is wrong in the input mesh at cell #" << i << " ! The size of cell is not >=0 !";
              throw INTERP_KERNEL::Exception(oss.str().c_str());
            }
        }
      else
        {
          std::ostringstream oss; oss << "MEDCoupling1DGTUMesh::New(const MEDCouplingUMesh *m) : something is wrong in the input mesh at cell #" << i << " ! The geometric type is not those expected !";
          throw INTERP_KERNEL::Exception(oss.str().c_str());
        }
    }
  ret->setNodalConnectivity(conn,connI);
  return ret.retn();
}
