/** \file
 *
 *  $Date: 2008/04/10 16:36:41 $
 *  $Revision: 1.7 $
 *  \author Andre Sznajder - UERJ(Brazil)
 */
 

#include <memory>

#include "Alignment/MuonAlignment/interface/AlignableDTBarrel.h"
#include "CondFormats/Alignment/interface/Alignments.h" 
#include "CondFormats/Alignment/interface/AlignmentErrorsExtended.h" 
#include "CondFormats/Alignment/interface/AlignmentSorter.h" 
#include "FWCore/MessageLogger/interface/MessageLogger.h"


/// The constructor simply copies the vector of wheels and computes the surface from them
AlignableDTBarrel::AlignableDTBarrel( const std::vector<AlignableDTWheel*>& dtWheels ) 
   : AlignableComposite(dtWheels[0]->id(), align::AlignableDTBarrel)
{

  theDTWheels.insert( theDTWheels.end(), dtWheels.begin(), dtWheels.end() );

  // maintain also list of components
  for (const auto& wheel: dtWheels) {
    const auto mother = wheel->mother();
    this->addComponent(wheel); // components will be deleted by dtor of AlignableComposite
    wheel->setMother(mother); // restore previous behaviour where mother is not set
  }

  setSurface( computeSurface() );
  compConstraintType_ = Alignable::CompConstraintType::POSITION_Z;
}


/// Return AlignableBarrelLayer at given index
AlignableDTWheel &AlignableDTBarrel::wheel(int i) 
{
  
  if (i >= size() ) 
	throw cms::Exception("LogicError") << "Wheel index (" << i << ") out of range";

  return *theDTWheels[i];
  
}


/// Returns surface corresponding to current position
/// and orientation, as given by average on all components
AlignableSurface AlignableDTBarrel::computeSurface()
{

  return AlignableSurface( computePosition(), computeOrientation() );

}



/// Compute average z position from all components (x and y forced to 0)
AlignableDTBarrel::PositionType AlignableDTBarrel::computePosition() 
{

  float zz = 0.;

  for ( std::vector<AlignableDTWheel*>::iterator ilayer = theDTWheels.begin();
		ilayer != theDTWheels.end(); ilayer++ )
    zz += (*ilayer)->globalPosition().z();

  zz /= static_cast<float>(theDTWheels.size());

  return PositionType( 0.0, 0.0, zz );

}


/// Just initialize to default given by default constructor of a RotationType
AlignableDTBarrel::RotationType AlignableDTBarrel::computeOrientation() 
{
  return RotationType();
}



/// Output Half Barrel information
std::ostream &operator << (std::ostream& os, const AlignableDTBarrel& b )
{

  os << "This DTBarrel contains " << b.theDTWheels.size() << " Barrel wheels" << std::endl;
  os << "(phi, r, z) =  (" << b.globalPosition().phi() << "," 
     << b.globalPosition().perp() << "," << b.globalPosition().z();
  os << "),  orientation:" << std::endl<< b.globalRotation() << std::endl;
  return os;

}


/// Recursive printout of whole Half Barrel structure
void AlignableDTBarrel::dump( void ) const
{

  edm::LogInfo("AlignableDump") << (*this);
  for ( std::vector<AlignableDTWheel*>::const_iterator iWheel = theDTWheels.begin();
		iWheel != theDTWheels.end(); iWheel++ )
	(*iWheel)->dump();

}

//__________________________________________________________________________________________________
Alignments* AlignableDTBarrel::alignments( void ) const
{
  Alignments* m_alignments = new Alignments();

  // Add components recursively
  for (const auto& i: this->components()) {
    std::unique_ptr<Alignments> tmpAlignments{i->alignments()};
    std::copy(tmpAlignments->m_align.begin(), tmpAlignments->m_align.end(),
              std::back_inserter(m_alignments->m_align));
  }

  std::sort( m_alignments->m_align.begin(), m_alignments->m_align.end(), 
			 lessAlignmentDetId<AlignTransform>() );

  return m_alignments;
}

//__________________________________________________________________________________________________
AlignmentErrorsExtended* AlignableDTBarrel::alignmentErrors( void ) const
{
  AlignmentErrorsExtended* m_alignmentErrors = new AlignmentErrorsExtended();

  // Add components recursively
  for (const auto& i: this->components()) {
    std::unique_ptr<AlignmentErrorsExtended> tmpAlignmentErrorsExtended{i->alignmentErrors()};
    std::copy(tmpAlignmentErrorsExtended->m_alignError.begin(), tmpAlignmentErrorsExtended->m_alignError.end(),
              std::back_inserter(m_alignmentErrors->m_alignError));
  }

  std::sort( m_alignmentErrors->m_alignError.begin(), m_alignmentErrors->m_alignError.end(), 
			 lessAlignmentDetId<AlignTransformErrorExtended>() );

  return m_alignmentErrors;
}


