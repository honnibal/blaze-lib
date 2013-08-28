//=================================================================================================
/*!
//  \file blaze/math/views/SparseSubvector.h
//  \brief Header file for the SparseSubvector class template
//
//  Copyright (C) 2011 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. This library is free software; you can redistribute
//  it and/or modify it under the terms of the GNU General Public License as published by the
//  Free Software Foundation; either version 3, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with a special
//  exception for linking and compiling against the Blaze library, the so-called "runtime
//  exception"; see the file COPYING. If not, see http://www.gnu.org/licenses/.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_VIEWS_SPARSESUBVECTOR_H_
#define _BLAZE_MATH_VIEWS_SPARSESUBVECTOR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iterator>
#include <stdexcept>
#include <blaze/math/constraints/Computation.h>
#include <blaze/math/constraints/DenseVector.h>
#include <blaze/math/constraints/RequiresEvaluation.h>
#include <blaze/math/constraints/SparseVector.h>
#include <blaze/math/constraints/TransExpr.h>
#include <blaze/math/constraints/TransposeFlag.h>
#include <blaze/math/expressions/Expression.h>
#include <blaze/math/expressions/SparseVector.h>
#include <blaze/math/Forward.h>
#include <blaze/math/shims/IsDefault.h>
#include <blaze/math/traits/AddTrait.h>
#include <blaze/math/traits/DivTrait.h>
#include <blaze/math/traits/MultTrait.h>
#include <blaze/math/traits/SubTrait.h>
#include <blaze/math/traits/SubvectorExprTrait.h>
#include <blaze/math/traits/SubvectorTrait.h>
#include <blaze/math/typetraits/IsComputation.h>
#include <blaze/math/typetraits/IsTransExpr.h>
#include <blaze/math/typetraits/IsTransposeVector.h>
#include <blaze/math/typetraits/IsVecAbsExpr.h>
#include <blaze/math/typetraits/IsVecEvalExpr.h>
#include <blaze/math/typetraits/IsVecScalarDivExpr.h>
#include <blaze/math/typetraits/IsVecScalarMultExpr.h>
#include <blaze/math/typetraits/IsVecTransExpr.h>
#include <blaze/math/typetraits/IsVecVecAddExpr.h>
#include <blaze/math/typetraits/IsVecVecMultExpr.h>
#include <blaze/math/typetraits/IsVecVecSubExpr.h>
#include <blaze/util/Assert.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/logging/FunctionTrace.h>
#include <blaze/util/mpl/If.h>
#include <blaze/util/mpl/Or.h>
#include <blaze/util/SelectType.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/IsConst.h>
#include <blaze/util/typetraits/IsFloatingPoint.h>
#include <blaze/util/typetraits/IsNumeric.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup sparse_subvector Subvector
// \ingroup views
*/
/*!\brief View to a specific subvector of a sparse vector.
// \ingroup sparse_subvector
//
// The SparseSubvector template represents a view to a specific subvector of a sparse vector
// primitive. The type of the sparse vector is specified via the first template parameter:

   \code
   template< typename VT, bool TF >
   class SparseSubvector;
   \endcode

//  - VT: specifies the type of the sparse vector primitive. SparseSubvector can be used with any
//        sparse vector primitive or view, but does not work with any vector expression type.
//  - TF: specifies whether the vector is a row vector (\a blaze::rowVector) or a column
//        vector (\a blaze::columnVector). This template parameter doesn't have to be explicitly
//        defined, but is automatically derived from the first template parameter.
//
//
// \n \section sparse_subvector_setup Setup of Sparse Subvectors
//
// A view to a sparse subvector can very conveniently be created via the \c sub() function. This
// view can be treated as any other sparse vector, i.e. it can be assigned to, it can be copied
// from, and it can be used in arithmetic operations. The view can also be used on both sides of
// an assignment: The subvector can be either used as an alias to grant write access to a specific
// subvector of a sparse vector primitive on the left-hand side of an assignment or to grant
// read-access to a specific subvector of a sparse vector primitive or expression on the right-hand
// side of an assignment. The following example demonstrates this in detail:

   \code
   typedef blaze::DynamicVector<double,rowVector>     DenseVectorType;
   typedef blaze::CompressedVector<double,rowVector>  SparseVectorType;
   typedef blaze::CompressedMatrix<double,rowMajor>   SparseMatrixType;

   DenseVectorType  x;
   SparseVectorType y;
   SparseMatrixType A;
   // ... Resizing and initialization

   // Setting the first ten elements of y to the 2nd row of matrix A
   blaze::SparseSubvector<SparseVectorType> sv = sub( y, 0UL, 10UL );
   sv = row( A, 2UL );

   // Setting the second ten elements of y to x
   sub( y, 10UL, 10UL ) = x;

   // Setting the 3rd row of A to a subvector of y
   row( A, 3UL ) = sub( y, 3UL, 10UL );

   // Setting y to a subvector of the result of the addition between x and the 1st row of A
   y = sub( x + row( A, 1UL ), 2UL, 5UL )
   \endcode

// \n \section sparse_subvector_element_access Element access
//
// A sparse subvector can be used like any other sparse vector. For instance, the elements of the
// sparse subvector can be directly accessed with the subscript operator. The numbering of the
// subvector elements is

                             \f[\left(\begin{array}{*{5}{c}}
                             0 & 1 & 2 & \cdots & N-1 \\
                             \end{array}\right),\f]

// where N is the specified size of the subvector. Alternatively, the elements of a subvector can
// be traversed via iterators. Just as with vectors, in case of non-const subvectors, \c begin()
// and \c end() return an Iterator, which allows a manipulation of the non-zero values, in case
// of constant subvectors a ConstIterator is returned:

   \code
   typedef blaze::CompressedVector<int,rowVector>  VectorType;
   typedef blaze::SparseSubvector<VectorType>      SubvectorType;

   VectorType v( 256UL );
   // ... Resizing and initialization

   // Creating a reference to a specific subvector of vector v
   SubvectorType sv = sub( v, 16UL, 64UL );

   for( SubvectorType::Iterator it=sv.begin(); it!=sv.end(); ++it ) {
      it->value() = ...;  // OK: Write access to the value of the non-zero element.
      ... = it->value();  // OK: Read access to the value of the non-zero element.
      it->index() = ...;  // Compilation error: The index of a non-zero element cannot be changed.
      ... = it->index();  // OK: Read access to the index of the sparse element.
   }

   for( SubvectorType::ConstIterator it=sv.begin(); it!=sv.end(); ++it ) {
      it->value() = ...;  // Compilation error: Assignment to the value via a ConstIterator is invalid.
      ... = it->value();  // OK: Read access to the value of the non-zero element.
      it->index() = ...;  // Compilation error: The index of a non-zero element cannot be changed.
      ... = it->index();  // OK: Read access to the index of the sparse element.
   }
   \endcode

// \n \section sparse_subvector_element_insertion Element Insertion
//
// Inserting/accessing elements in a sparse subvector can be done by several alternative functions.
// The following example demonstrates all options:

   \code
   using blaze::CompressedVector;
   using blaze::SparseSubvector;

   typedef CompressedVector<double,rowVector>  VectorType;
   VectorType v( 256UL );  // Non-initialized vector of size 256

   typedef SparseSubvector<VectorType>  SubvectorType;
   SubvectorType sv( sub( v, 10UL, 60UL ) );  // View to the range [10..69] of v

   // The subscript operator provides access to all possible elements of the sparse subvector,
   // including the zero elements. In case the subscript operator is used to access an element
   // that is currently not stored in the sparse subvector, the element is inserted into the
   // subvector.
   sv[42] = 2.0;

   // An alternative for inserting elements into the subvector is the insert() function. However,
   // it inserts the element only in case the element is not already contained in the subvector.
   sv.insert( 50UL, 3.7 );

   // As well as in the case of vectors, elements can also be inserted via the append() function.
   // Also in case of subvectors, append() requires that the appended element's index is strictly
   // larger than the currently largest non-zero index of the subvector and that the subvector's
   // capacity is large enough to hold the new element. Note however that due to the nature of
   // a subvector, which may be an alias to the middle of a sparse vector, the append() function
   // is not as efficient for a subvector as it is for a vector.
   sv.reserve( 10UL );
   sv.append( 51UL, -2.1 );

   // In order to traverse all non-zero elements currently stored in the subvector, the begin()
   // and end() functions can be used. In the example, all non-zero elements of the subvector
   // are traversed.
   for( SubvectorType::Iterator it=sv.begin(); it!=a.end(); ++it ) {
      ... = it->value();  // Access to the value of the non-zero element
      ... = it->index();  // Access to the index of the non-zero element
   }
   \endcode

// \n \section sparse_subvector_common_operations Common Operations
//
// The current number of subvector elements can be obtained via the \c size() function, the
// current capacity via the \c capacity() function, and the number of non-zero elements via
// the \c nonZeros() function. However, since subvector are views to a specific subvector of
// a vector, several operations are not possible on views, such as resizing and swapping:

   \code
   typedef blaze::CompressedVector<int,rowVector>  VectorType;
   typedef blaze::SparseSubvector<VectorType>      SubvectorType;

   VectorType v( 42UL );
   // ... Resizing and initialization

   // Creating a view to the range [5..15] of vector v
   SubvectorType sv = sub( v, 5UL, 10UL );

   sv.size();          // Returns the number of elements in the subvector
   sv.capacity();      // Returns the capacity of the subvector
   sv.nonZeros();      // Returns the number of non-zero elements contained in the subvector

   sv.resize( 84UL );  // Compilation error: Cannot resize a subvector of a vector

   SubvectorType sv2 = sub( v, 15UL, 10UL );
   swap( sv, sv2 );   // Compilation error: Swap operation not allowed
   \endcode

// \n \section sparse_subvector_arithmetic_operations Arithmetic Operations
//
// The following example gives an impression of the use of SparseSubvector within arithmetic
// operations. All operations (addition, subtraction, multiplication, scaling, ...) can be
// performed on all possible combinations of dense and sparse vectors with fitting element
// types:

   \code
   using blaze::DynamicVector;
   using blaze::CompressedVector;
   using blaze::CompressedMatrix;
   using blaze::SparseSubvector;

   typedef CompressedVector<double,rowVector>  SparseVectorType;
   typedef DynamicVector<double,rowVector>     DenseVectorType;
   SparseVectorType s1, s2, s3;
   DenseVectorType d1, d2;

   // ... Resizing and initialization

   typedef CompressedMatrix<double,rowMajor>  SparseMatrixType;
   SparseMatrixType A;

   typedef SparseSubvector<SparseVectorType>  SubvectorType;
   SubvectorType sv1( sub( s1,  0UL, 10UL ) );  // View on the range [0..9] of vector s1
   SubvectorType sv2( sub( s1, 20UL, 10UL ) );  // View on the range [20..29] of vector s1

   sv1[1] = 2.0;                 // Manual initialization of values in the range [0..9]
   sv1[5] = 3.0;
   sub( s1, 10UL, 10UL ) = s2;   // Sparse vector initialization of the range [10..19]
   sv2 = d1;                     // Dense vector initialization of the range [20..29]

   s3 = sv1 + s2;                      // Sparse vector/sparse vector addition
   d2 = d1  + sub( s1, 10UL, 10UL );   // Dense vector/sparse vector addition
   s2 = sv1 * sub( s1, 20UL, 10UL );   // Component-wise vector multiplication

   sub( s1, 3UL, 4UL ) *= 2.0;     // In-place scaling of the range [3..6]
   b = sub( s1, 7UL, 3UL ) * 2.0;  // Scaling of the range [7..9]
   b = 2.0 * sub( s1, 7UL, 3UL );  // Scaling of the range [7..9]

   sub( s1, 0UL , 10UL ) += a;    // Addition assignment
   sub( s1, 10UL, 10UL ) -= c;    // Subtraction assignment
   sub( s1, 20UL, 10UL ) *= sv1;  // Multiplication assignment

   double scalar = sub( s1, 5UL, 10UL ) * trans( d1 );  // Scalar/dot/inner product between two vectors

   A = trans( d1 ) * sub( s1, 4UL, 16UL );  // Outer product between two vectors
   \endcode
*/
template< typename VT                               // Type of the sparse vector
        , bool TF = IsTransposeVector<VT>::value >  // Transpose flag
class SparseSubvector : public SparseVector< SparseSubvector<VT,TF>, TF >
                      , private Expression
{
 private:
   //**********************************************************************************************
   //! Compilation switch for the non-const reference and iterator types.
   /*! The \a useConst compile time constant expression represents a compilation switch for
       the non-const reference and iterator types. In case the given sparse vector of type
       \a VT is const qualified, \a useConst will be set to 1 and the sparse subvector will
       return references and iterators to const. Otherwise \a useConst will be set to 0 and
       the sparse subvector will offer write access to the sparse matrix elements both via
       the subscript operator and iterators. */
   enum { useConst = IsConst<VT>::value };
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef SparseSubvector<VT,TF>              This;           //!< Type of this SparseSubvector instance.
   typedef typename SubvectorTrait<VT>::Type   ResultType;     //!< Result type for expression template evaluations.
   typedef typename ResultType::TransposeType  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef typename VT::ElementType            ElementType;    //!< Type of the subvector elements.
   typedef typename VT::ReturnType             ReturnType;     //!< Return type for expression template evaluations
   typedef const SparseSubvector&              CompositeType;  //!< Data type for composite expression templates.

   //! Reference to a constant subvector value.
   typedef typename VT::ConstReference  ConstReference;

   //! Reference to a non-constant subvector value.
   typedef typename SelectType< useConst, ConstReference, typename VT::Reference >::Type  Reference;
   //**********************************************************************************************

   //**SubvectorElement class definition***********************************************************
   /*!\brief Access proxy for a specific element of the sparse subvector.
   */
   template< typename IteratorType  // Type of the sparse vector iterator
           , bool ConstFlag >       // Constness flag
   class SubvectorElement
   {
    public:
      //**Type definitions*************************************************************************
      typedef typename SelectType< ConstFlag, const ElementType&, ElementType& >::Type  ReferenceType;
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the SubvectorElement class.
      //
      // \param pos Iterator to the current position within the sparse subvector.
      // \param offset The offset within the according sparse vector.
      */
      inline SubvectorElement( IteratorType pos, size_t offset )
         : pos_   ( pos    )  // Iterator to the current position within the sparse subvector
         , offset_( offset )  // Offset within the according sparse vector
      {
         BLAZE_INTERNAL_ASSERT( pos_->index() >= offset_, "Invalid offset detected" );
      }
      //*******************************************************************************************

      //**Assignment operator**********************************************************************
      /*!\brief Assignment to the accessed sparse subvector element.
      //
      // \param value The new value of the sparse subvector element.
      // \return Reference to the sparse subvector element.
      */
      template< typename T > inline SubvectorElement& operator=( const T& v ) {
         *pos_ = v;
         return *this;
      }
      //*******************************************************************************************

      //**Addition assignment operator*************************************************************
      /*!\brief Addition assignment to the accessed sparse subvector element.
      //
      // \param value The right-hand side value for the addition.
      // \return Reference to the sparse subvector element.
      */
      template< typename T > inline SubvectorElement& operator+=( const T& v ) {
         *pos_ += v;
         return *this;
      }
      //*******************************************************************************************

      //**Subtraction assignment operator**********************************************************
      /*!\brief Subtraction assignment to the accessed sparse subvector element.
      //
      // \param value The right-hand side value for the subtraction.
      // \return Reference to the sparse subvector element.
      */
      template< typename T > inline SubvectorElement& operator-=( const T& v ) {
         *pos_ -= v;
         return *this;
      }
      //*******************************************************************************************

      //**Multiplication assignment operator*******************************************************
      /*!\brief Multiplication assignment to the accessed sparse subvector element.
      //
      // \param value The right-hand side value for the multiplication.
      // \return Reference to the sparse subvector element.
      */
      template< typename T > inline SubvectorElement& operator*=( const T& v ) {
         *pos_ *= v;
         return *this;
      }
      //*******************************************************************************************

      //**Division assignment operator*************************************************************
      /*!\brief Division assignment to the accessed sparse subvector element.
      //
      // \param value The right-hand side value for the division.
      // \return Reference to the sparse subvector element.
      */
      template< typename T > inline SubvectorElement& operator/=( const T& v ) {
         *pos_ /= v;
         return *this;
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the sparse vector element at the current iterator position.
      //
      // \return Reference to the sparse vector element at the current iterator position.
      */
      inline const SubvectorElement* operator->() const {
         return this;
      }
      //*******************************************************************************************

      //**Value function***************************************************************************
      /*!\brief Access to the current value of the sparse subvector element.
      //
      // \return The current value of the sparse subvector element.
      */
      inline ReferenceType value() const {
         return pos_->value();
      }
      //*******************************************************************************************

      //**Index function***************************************************************************
      /*!\brief Access to the current index of the sparse element.
      //
      // \return The current index of the sparse element.
      */
      inline size_t index() const {
         return pos_->index() - offset_;
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      IteratorType pos_;  //!< Iterator to the current position within the sparse subvector.
      size_t offset_;     //!< Offset within the according sparse vector.
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**SubvectorIterator class definition**********************************************************
   /*!\brief Iterator over the elements of the sparse subvector.
   */
   template< typename IteratorType  // Type of the sparse vector iterator
           , bool ConstFlag >       // Constness flag
   class SubvectorIterator
   {
    public:
      //**Type definitions*************************************************************************
      typedef std::forward_iterator_tag                 IteratorCategory;  //!< The iterator category.
      typedef SubvectorElement<IteratorType,ConstFlag>  ValueType;         //!< Type of the underlying elements.
      typedef ValueType                                 PointerType;       //!< Pointer return type.
      typedef ValueType                                 ReferenceType;     //!< Reference return type.
      typedef ptrdiff_t                                 DifferenceType;    //!< Difference between two iterators.

      // STL iterator requirements
      typedef IteratorCategory  iterator_category;  //!< The iterator category.
      typedef ValueType         value_type;         //!< Type of the underlying elements.
      typedef PointerType       pointer;            //!< Pointer return type.
      typedef ReferenceType     reference;          //!< Reference return type.
      typedef DifferenceType    difference_type;    //!< Difference between two iterators.
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Constructor for the SubvectorIterator class.
      //
      // \param pos Iterator to the current sparse element.
      // \param offset The offset of the subvector within the sparse vector.
      */
      inline SubvectorIterator( IteratorType pos, size_t offset )
         : pos_   ( pos    )  // Iterator to the current sparse element
         , offset_( offset )  // The offset of the subvector within the sparse vector
      {}
      //*******************************************************************************************

      //**Constructor******************************************************************************
      /*!\brief Conversion constructor from different SubvectorIterator instances.
      //
      // \param it The subvector iterator to be copied.
      */
      template< typename IteratorType2, bool ConstFlag2 >
      inline SubvectorIterator( const SubvectorIterator<IteratorType2,ConstFlag2>& it )
         : pos_   ( it.pos_    )  // Iterator to the current sparse element.
         , offset_( it.offset_ )  // The offset of the subvector within the sparse vector
      {}
      //*******************************************************************************************

      //**Prefix increment operator****************************************************************
      /*!\brief Pre-increment operator.
      //
      // \return Reference to the incremented iterator.
      */
      inline SubvectorIterator& operator++() {
         ++pos_;
         return *this;
      }
      //*******************************************************************************************

      //**Postfix increment operator***************************************************************
      /*!\brief Post-increment operator.
      //
      // \return The previous position of the iterator.
      */
      inline const SubvectorIterator operator++( int ) {
         const SubvectorIterator tmp( *this );
         ++(*this);
         return tmp;
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the sparse vector element at the current iterator position.
      //
      // \return The current value of the sparse element.
      */
      inline ReferenceType operator*() const {
         return ReferenceType( pos_, offset_ );
      }
      //*******************************************************************************************

      //**Element access operator******************************************************************
      /*!\brief Direct access to the sparse vector element at the current iterator position.
      //
      // \return Reference to the sparse vector element at the current iterator position.
      */
      inline PointerType operator->() const {
         return PointerType( pos_, offset_ );
      }
      //*******************************************************************************************

      //**Equality operator************************************************************************
      /*!\brief Equality comparison between two SubvectorIterator objects.
      //
      // \param rhs The right-hand side expression iterator.
      // \return \a true if the iterators refer to the same element, \a false if not.
      */
      template< typename IteratorType2, bool ConstFlag2 >
      inline bool operator==( const SubvectorIterator<IteratorType2,ConstFlag2>& rhs ) const {
         return pos_ == rhs.pos_;
      }
      //*******************************************************************************************

      //**Inequality operator**********************************************************************
      /*!\brief Inequality comparison between two SubvectorIterator objects.
      //
      // \param rhs The right-hand side expression iterator.
      // \return \a true if the iterators don't refer to the same element, \a false if they do.
      */
      template< typename IteratorType2, bool ConstFlag2 >
      inline bool operator!=( const SubvectorIterator<IteratorType2,ConstFlag2>& rhs ) const {
         return !( *this == rhs );
      }
      //*******************************************************************************************

      //**Subtraction operator*********************************************************************
      /*!\brief Calculating the number of elements between two subvector iterators.
      //
      // \param rhs The right-hand side subvector iterator.
      // \return The number of elements between the two subvector iterators.
      */
      inline DifferenceType operator-( const SubvectorIterator& rhs ) const {
         return pos_ - rhs.pos_;
      }
      //*******************************************************************************************

    private:
      //**Member variables*************************************************************************
      IteratorType pos_;     //!< Iterator to the current sparse element.
      size_t       offset_;  //!< The offset of the subvector within the sparse vector.
      //*******************************************************************************************

      //**Friend declarations**********************************************************************
      /*! \cond BLAZE_INTERNAL */
      template< typename IteratorType2, bool ConstFlag2 > friend class SubvectorIterator;
      template< typename VT2, bool TF2 > friend class SparseSubvector;
      /*! \endcond */
      //*******************************************************************************************
   };
   //**********************************************************************************************

   //**Type definitions****************************************************************************
   //! Iterator over constant elements.
   typedef SubvectorIterator<typename VT::ConstIterator,true>  ConstIterator;

   //! Iterator over non-constant elements.
   typedef typename SelectType< useConst, ConstIterator, SubvectorIterator<typename VT::Iterator,false> >::Type  Iterator;
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit inline SparseSubvector( VT& vector, size_t start, size_t n );
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   inline Reference      operator[]( size_t index );
   inline ConstReference operator[]( size_t index ) const;
   inline Iterator       begin ();
   inline ConstIterator  begin () const;
   inline ConstIterator  cbegin() const;
   inline Iterator       end   ();
   inline ConstIterator  end   () const;
   inline ConstIterator  cend  () const;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
                            inline SparseSubvector& operator= ( const SparseSubvector& rhs );
   template< typename VT2 > inline SparseSubvector& operator= ( const Vector<VT2,TF>& rhs );
   template< typename VT2 > inline SparseSubvector& operator+=( const Vector<VT2,TF>& rhs );
   template< typename VT2 > inline SparseSubvector& operator-=( const Vector<VT2,TF>& rhs );
   template< typename VT2 > inline SparseSubvector& operator*=( const Vector<VT2,TF>& rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SparseSubvector >::Type&
      operator*=( Other rhs );

   template< typename Other >
   inline typename EnableIf< IsNumeric<Other>, SparseSubvector >::Type&
      operator/=( Other rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
                              inline size_t           size() const;
                              inline size_t           capacity() const;
                              inline size_t           nonZeros() const;
                              inline void             reset();
                              inline ElementType&     insert ( size_t index, const ElementType& value );
                              inline void             erase  ( size_t index );
                              inline Iterator         erase  ( Iterator pos );
                              inline Iterator         erase  ( Iterator first, Iterator last );
                              inline void             reserve( size_t n );
   template< typename Other > inline SparseSubvector& scale  ( Other scalar );
   //@}
   //**********************************************************************************************

   //**Lookup functions****************************************************************************
   /*!\name Lookup functions */
   //@{
   inline Iterator      find      ( size_t index );
   inline ConstIterator find      ( size_t index ) const;
   inline Iterator      lowerBound( size_t index );
   inline ConstIterator lowerBound( size_t index ) const;
   inline Iterator      upperBound( size_t index );
   inline ConstIterator upperBound( size_t index ) const;
   //@}
   //**********************************************************************************************

   //**Low-level utility functions*****************************************************************
   /*!\name Low-level utility functions */
   //@{
   inline void append( size_t index, const ElementType& value, bool check=false );
   //@}
   //**********************************************************************************************

   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other > inline bool canAlias ( const Other* alias ) const;
   template< typename Other > inline bool isAliased( const Other* alias ) const;
   template< typename VT2 >   inline void assign   ( const DenseVector <VT2,TF>& rhs );
   template< typename VT2 >   inline void assign   ( const SparseVector<VT2,TF>& rhs );
   template< typename VT2 >   inline void addAssign( const DenseVector <VT2,TF>& rhs );
   template< typename VT2 >   inline void addAssign( const SparseVector<VT2,TF>& rhs );
   template< typename VT2 >   inline void subAssign( const DenseVector <VT2,TF>& rhs );
   template< typename VT2 >   inline void subAssign( const SparseVector<VT2,TF>& rhs );
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   VT&          vector_;  //!< The sparse vector containing the subvector.
   const size_t start_;   //!< The first index of the subvector.
   const size_t size_;    //!< The size of the subvector.
   //@}
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE  ( VT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_TRANSEXPR_TYPE  ( VT );
   BLAZE_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG( VT, TF );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The constructor for SparseSubvector.
//
// \param vector The sparse vector containing the subvector.
// \param start The first index of the subvector in the given sparse vector.
// \param n The size of the subvector.
// \exception std::invalid_argument Invalid subvector specification.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline SparseSubvector<VT,TF>::SparseSubvector( VT& vector, size_t start, size_t n )
   : vector_( vector )  // The sparse vector containing the subvector
   , start_ ( start  )  // The first index of the subvector
   , size_  ( n      )  // The size of the subvector
{
   if( n == 0UL || start + n > vector.size() )
      throw std::invalid_argument( "Invalid subvector specification" );
}
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Subscript operator for the direct access to the subvector elements.
//
// \param index Access index. The index must be smaller than the number of subvector elements.
// \return Reference to the accessed value.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Reference
   SparseSubvector<VT,TF>::operator[]( size_t index )
{
   BLAZE_USER_ASSERT( index < size(), "Invalid subvector access index" );
   return vector_[start_+index];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subscript operator for the direct access to the subvector elements.
//
// \param index Access index. The index must be smaller than the number of matrix columns.
// \return Reference to the accessed value.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstReference
   SparseSubvector<VT,TF>::operator[]( size_t index ) const
{
   BLAZE_USER_ASSERT( index < size(), "Invalid subvector access index" );
   return const_cast<const VT&>( vector_ )[start_+index];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the subvector.
//
// \return Iterator to the first element of the subvector.
//
// This function returns an iterator to the first element of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::begin()
{
   return Iterator( vector_.lowerBound( start_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the subvector.
//
// \return Iterator to the first element of the subvector.
//
// This function returns an iterator to the first element of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::begin() const
{
   return ConstIterator( vector_.lowerBound( start_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the subvector.
//
// \return Iterator to the first element of the subvector.
//
// This function returns an iterator to the first element of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::cbegin() const
{
   return ConstIterator( vector_.lowerBound( start_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the subvector.
//
// \return Iterator just past the last element of the subvector.
//
// This function returns an iterator just past the last element of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::end()
{
   return Iterator( vector_.lowerBound( start_ + size_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the subvector.
//
// \return Iterator just past the last element of the subvector.
//
// This function returns an iterator just past the last element of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::end() const
{
   return ConstIterator( vector_.lowerBound( start_ + size_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the subvector.
//
// \return Iterator just past the last element of the subvector.
//
// This function returns an iterator just past the last element of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::cend() const
{
   return ConstIterator( vector_.lowerBound( start_ + size_ ), start_ );
}
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for SparseSubvector.
//
// \param rhs Sparse subvector to be copied.
// \return Reference to the assigned subvector.
// \exception std::invalid_argument Subvector sizes do not match.
//
// In case the current sizes of the two subvectors don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline SparseSubvector<VT,TF>& SparseSubvector<VT,TF>::operator=( const SparseSubvector& rhs )
{
   using blaze::assign;

   if( this == &rhs || ( &vector_ == &rhs.vector_ && start_ == rhs.start_ ) )
      return *this;

   if( size() != rhs.size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   if( rhs.canAlias( &vector_ ) ) {
      const ResultType tmp( rhs );
      reset();
      assign( *this, tmp );
   }
   else {
      reset();
      assign( *this, rhs );
   }

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Assignment operator for different vectors.
//
// \param rhs Vector to be assigned.
// \return Reference to the assigned subvector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument
// exception is thrown.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side vector
inline SparseSubvector<VT,TF>& SparseSubvector<VT,TF>::operator=( const Vector<VT2,TF>& rhs )
{
   using blaze::assign;

   if( size() != (~rhs).size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   if( (~rhs).canAlias( &vector_ ) ) {
      const typename VT2::ResultType tmp( ~rhs );
      reset();
      assign( *this, tmp );
   }
   else {
      reset();
      assign( *this, ~rhs );
   }

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the sparse subvector.
// \return Reference to the sparse subvector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side vector
inline SparseSubvector<VT,TF>& SparseSubvector<VT,TF>::operator+=( const Vector<VT2,TF>& rhs )
{
   using blaze::addAssign;

   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   addAssign( *this, ~rhs );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the sparse subvector.
// \return Reference to the sparse subvector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side vector
inline SparseSubvector<VT,TF>& SparseSubvector<VT,TF>::operator-=( const Vector<VT2,TF>& rhs )
{
   using blaze::subAssign;

   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   subAssign( *this, ~rhs );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication of a vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be multiplied with the sparse subvector.
// \return Reference to the sparse subvector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side vector
inline SparseSubvector<VT,TF>& SparseSubvector<VT,TF>::operator*=( const Vector<VT2,TF>& rhs )
{
   if( (~rhs).size() != size() )
      throw std::invalid_argument( "Vector sizes do not match" );

   typedef typename MultTrait<ResultType,typename VT2::ResultType>::Type  MultType;

   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( MultType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( MultType );

   const MultType tmp( *this * (~rhs) );
   reset();
   assign( tmp );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication between a sparse subvector
//        and a scalar value (\f$ \vec{a}*=s \f$).
//
// \param rhs The right-hand side scalar value for the multiplication.
// \return Reference to the sparse subvector.
//
// This operator can only be used for built-in data types. Additionally, the elements of
// the sparse subvector must support the multiplication assignment operator for the given
// scalar built-in data type.
*/
template< typename VT       // Type of the sparse vector
        , bool TF >         // Transpose flag
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SparseSubvector<VT,TF> >::Type&
   SparseSubvector<VT,TF>::operator*=( Other rhs )
{
   for( Iterator element=begin(); element!=end(); ++element )
      element->value() *= rhs;
   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Division assignment operator for the division of a sparse subvector by a scalar value
//        (\f$ \vec{a}/=s \f$).
//
// \param rhs The right-hand side scalar value for the division.
// \return Reference to the sparse subvector.
//
// This operator can only be used for built-in data types. Additionally, the elements of the
// sparse subvector must either support the multiplication assignment operator for the given
// floating point data type or the division assignment operator for the given integral data
// type.
*/
template< typename VT       // Type of the sparse vector
        , bool TF >         // Transpose flag
template< typename Other >  // Data type of the right-hand side scalar
inline typename EnableIf< IsNumeric<Other>, SparseSubvector<VT,TF> >::Type&
   SparseSubvector<VT,TF>::operator/=( Other rhs )
{
   BLAZE_USER_ASSERT( rhs != Other(0), "Division by zero detected" );

   typedef typename DivTrait<ElementType,Other>::Type  DT;
   typedef typename If< IsNumeric<DT>, DT, Other >::Type  Tmp;

   // Depending on the two involved data types, an integer division is applied or a
   // floating point division is selected.
   if( IsNumeric<DT>::value && IsFloatingPoint<DT>::value ) {
      const Tmp tmp( Tmp(1)/static_cast<Tmp>( rhs ) );
      for( Iterator element=begin(); element!=end(); ++element )
         element->value() *= tmp;
   }
   else {
      for( Iterator element=begin(); element!=end(); ++element )
         element->value() /= rhs;
   }

   return *this;
}
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the current size/dimension of the sparse subvector.
//
// \return The size of the sparse subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline size_t SparseSubvector<VT,TF>::size() const
{
   return size_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the maximum capacity of the sparse subvector.
//
// \return The capacity of the sparse subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline size_t SparseSubvector<VT,TF>::capacity() const
{
   return size_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the number of non-zero elements in the subvector.
//
// \return The number of non-zero elements in the subvector.
//
// Note that the number of non-zero elements is always smaller than the size of the subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline size_t SparseSubvector<VT,TF>::nonZeros() const
{
   return end() - begin();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline void SparseSubvector<VT,TF>::reset()
{
   vector_.erase( vector_.lowerBound( start_ ), vector_.lowerBound( start_ + size_ ) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inserting an element into the sparse subvector.
//
// \param index The index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be inserted.
// \return Reference to the inserted value.
// \exception std::invalid_argument Invalid sparse subvector access index.
//
// This function inserts a new element into the sparse subvector. However, duplicate elements
// are not allowed. In case the sparse subvector already contains an element at index \a index,
// a \a std::invalid_argument exception is thrown.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ElementType&
   SparseSubvector<VT,TF>::insert( size_t index, const ElementType& value )
{
   return vector_.insert( start_ + index, value )->value();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Erasing an element from the sparse subvector.
//
// \param index The index of the element to be erased. The index has to be in the range \f$[0..N-1]\f$.
// \return void
//
// This function erases an element from the sparse subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline void SparseSubvector<VT,TF>::erase( size_t index )
{
   vector_.erase( start_ + index );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Erasing an element from the sparse subvector.
//
// \param pos Iterator to the element to be erased.
// \return void
//
// This function erases an element from the sparse subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::erase( Iterator pos )
{
   return Iterator( vector_.erase( pos.pos_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Erasing a range of elements from the sparse subvector.
//
// \param first Iterator to first element to be erased.
// \param last Iterator just past the last element to be erased.
// \return Iterator to the element after the erased element.
//
// This function erases a range of elements from the sparse subvector.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::erase( Iterator first, Iterator last )
{
   return Iterator( vector_.erase( first.pos_, last.pos_ ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Setting the minimum capacity of the sparse subvector.
//
// \param n The new minimum capacity of the sparse subvector.
// \return void
//
// This function increases the capacity of the sparse subvector to at least \a n elements. The
// current values of the subvector elements are preserved.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
void SparseSubvector<VT,TF>::reserve( size_t n )
{
   return;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scaling of the sparse subvector by the scalar value \a scalar (\f$ \vec{a}=\vec{b}*s \f$).
//
// \param scalar The scalar value for the subvector scaling.
// \return Reference to the sparse subvector.
*/
template< typename VT       // Type of the sparse vector
        , bool TF >         // Transpose flag
template< typename Other >  // Data type of the scalar value
inline SparseSubvector<VT,TF>& SparseSubvector<VT,TF>::scale( Other scalar )
{
   for( Iterator element=begin(); element!=end(); ++element )
      element->value() *= scalar;
   return *this;
}
//*************************************************************************************************




//=================================================================================================
//
//  LOOKUP FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Searches for a specific subvector element.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the sparse
// subvector. It specifically searches for the element with index \a index. In case the element
// is found, the function returns an iterator to the element. Otherwise an iterator just past
// the last non-zero element of the sparse subvector (the end() iterator) is returned. Note that
// the returned sparse subvector iterator is subject to invalidation due to inserting operations
// via the subscript operator or the insert() function!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::find( size_t index )
{
   const typename VT::Iterator pos( vector_.find( start_ + index ) );

   if( pos != vector_.end() )
      return Iterator( pos, start_ );
   else
      return end();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Searches for a specific subvector element.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the element in case the index is found, end() iterator otherwise.
//
// This function can be used to check whether a specific element is contained in the sparse
// subvector. It specifically searches for the element with index \a index. In case the element
// is found, the function returns an iterator to the element. Otherwise an iterator just past
// the last non-zero element of the sparse subvector (the end() iterator) is returned. Note that
// the returned sparse subvector iterator is subject to invalidation due to inserting operations
// via the subscript operator or the insert() function!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::find( size_t index ) const
{
   const typename VT::ConstIterator pos( vector_.find( start_ + index ) );

   if( pos != vector_.end() )
      return Iterator( pos, start_ );
   else
      return end();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first index not less then the given index.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index not less then the given index, end() iterator otherwise.
//
// This function returns an iterator to the first element with an index not less then the given
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned sparse subvector
// iterator is subject to invalidation due to inserting operations via the subscript operator
// or the insert() function!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::lowerBound( size_t index )
{
   return Iterator( vector_.lowerBound( start_ + index ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first index not less then the given index.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index not less then the given index, end() iterator otherwise.
//
// This function returns an iterator to the first element with an index not less then the given
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned sparse subvector
// iterator is subject to invalidation due to inserting operations via the subscript operator
// or the insert() function!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::lowerBound( size_t index ) const
{
   return ConstIterator( vector_.lowerBound( start_ + index ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first index greater then the given index.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index greater then the given index, end() iterator otherwise.
//
// This function returns an iterator to the first element with an index greater then the given
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned sparse subvector
// iterator is subject to invalidation due to inserting operations via the subscript operator
// or the insert() function!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::Iterator SparseSubvector<VT,TF>::upperBound( size_t index )
{
   return Iterator( vector_.upperBound( start_ + index ), start_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first index greater then the given index.
//
// \param index The index of the search element. The index has to be in the range \f$[0..N-1]\f$.
// \return Iterator to the first index greater then the given index, end() iterator otherwise.
//
// This function returns an iterator to the first element with an index greater then the given
// index. In combination with the upperBound() function this function can be used to create a
// pair of iterators specifying a range of indices. Note that the returned sparse subvector
// iterator is subject to invalidation due to inserting operations via the subscript operator
// or the insert() function!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename SparseSubvector<VT,TF>::ConstIterator SparseSubvector<VT,TF>::upperBound( size_t index ) const
{
   return ConstIterator( vector_.upperBound( start_ + index ), start_ );
}
//*************************************************************************************************




//=================================================================================================
//
//  LOW-LEVEL UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Appending an element to the sparse subvector.
//
// \param index The index of the new element. The index has to be in the range \f$[0..N-1]\f$.
// \param value The value of the element to be appended.
// \param check \a true if the new value should be checked for default values, \a false if not.
// \return void
//
// This function provides a very efficient way to fill a sparse subvector with elements. It
// appends a new element to the end of the sparse subvector without any memory allocation.
// Therefore it is strictly necessary to keep the following preconditions in mind:
//
//  - the index of the new element must be strictly larger than the largest index of non-zero
//    elements in the sparse subvector
//  - the current number of non-zero elements must be smaller than the capacity of the subvector
//
// Ignoring these preconditions might result in undefined behavior! The optional \a check
// parameter specifies whether the new value should be tested for a default value. If the new
// value is a default value (for instance 0 in case of an integral element type) the value is
// not appended. Per default the values are not tested.
//
// \b Note: Although append() does not allocate new memory, it still invalidates all iterators
// returned by the end() functions!
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline void SparseSubvector<VT,TF>::append( size_t index, const ElementType& value, bool check )
{
   if( !check || !isDefault( value ) )
      vector_.insert( start_ + index, value );
}
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the sparse subvector can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this sparse subvector, \a false if not.
//
// This function returns whether the given address can alias with the sparse subvector. In
// contrast to the isAliased() function this function is allowed to use compile time expressions
// to optimize the evaluation.
*/
template< typename VT       // Type of the sparse vector
        , bool TF >         // Transpose flag
template< typename Other >  // Data type of the foreign expression
inline bool SparseSubvector<VT,TF>::canAlias( const Other* alias ) const
{
   return static_cast<const void*>( &vector_ ) == static_cast<const void*>( alias );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the sparse subvector is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this sparse subvector, \a false if not.
//
// This function returns whether the given address is aliased with the sparse subvector.
// In contrast to the canAlias() function this function is not allowed to use compile time
// expressions to optimize the evaluation.
*/
template< typename VT       // Type of the sparse vector
        , bool TF >         // Transpose flag
template< typename Other >  // Data type of the foreign expression
inline bool SparseSubvector<VT,TF>::isAliased( const Other* alias ) const
{
   return static_cast<const void*>( &vector_ ) == static_cast<const void*>( alias );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side dense vector
inline void SparseSubvector<VT,TF>::assign( const DenseVector<VT2,TF>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( size_t i=0UL; i<size(); ++i ) {
      append( i, (~rhs)[i], true );
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side sparse vector
inline void SparseSubvector<VT,TF>::assign( const SparseVector<VT2,TF>& rhs )
{
   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   for( typename VT2::ConstIterator element=(~rhs).begin(); element!=(~rhs).end(); ++element ) {
      append( element->index(), element->value() );
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side dense vector
inline void SparseSubvector<VT,TF>::addAssign( const DenseVector<VT2,TF>& rhs )
{
   typedef typename AddTrait<ResultType,typename VT2::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE    ( AddType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this + (~rhs) );
   reset();
   assign( tmp );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side sparse vector
inline void SparseSubvector<VT,TF>::addAssign( const SparseVector<VT2,TF>& rhs )
{
   typedef typename AddTrait<ResultType,typename VT2::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE   ( AddType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this + (~rhs) );
   reset();
   assign( tmp );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side dense vector
inline void SparseSubvector<VT,TF>::subAssign( const DenseVector<VT2,TF>& rhs )
{
   typedef typename AddTrait<ResultType,typename VT2::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE    ( AddType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this - (~rhs) );
   reset();
   assign( tmp );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename VT     // Type of the sparse vector
        , bool TF >       // Transpose flag
template< typename VT2 >  // Type of the right-hand side sparse vector
inline void SparseSubvector<VT,TF>::subAssign( const SparseVector<VT2,TF>& rhs )
{
   typedef typename AddTrait<ResultType,typename VT2::ResultType>::Type  AddType;

   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE   ( AddType );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( AddType );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION  ( AddType );

   BLAZE_INTERNAL_ASSERT( size() == (~rhs).size(), "Invalid vector sizes" );

   const AddType tmp( *this - (~rhs) );
   reset();
   assign( tmp );
}
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL FUNCTION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given sparse vector.
// \ingroup views
//
// \param sv The sparse vector containing the subvector.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specific subvector of the sparse vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given sparse
// vector. The following example demonstrates the creation of a subvector of size 8 starting
// from index 4:

   \code
   using blaze::columnVector;

   typedef blaze::CompressedVector<double,columnVector>  Vector;

   Vector v;
   // ... Resizing and initialization
   blaze::SparseSubvector<Vector> = sub( v, 4UL, 8UL );
   \endcode
*/
template< typename VT  // Type of the vector
        , bool TF >    // Transpose flag
inline typename DisableIf< Or< IsComputation<VT>, IsTransExpr<VT> >, SparseSubvector<VT> >::Type
   sub( SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return SparseSubvector<VT>( ~sv, start, n );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given sparse vector.
// \ingroup views
//
// \param sv The sparse vector containing the subvector.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specific subvector of the sparse vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given sparse
// vector. The following example demonstrates the creation of a subvector of size 8 starting
// from index 4:

   \code
   using blaze::columnVector;

   typedef blaze::CompressedVector<double,columnVector>  Vector;

   Vector v;
   // ... Resizing and initialization
   blaze::SparseSubvector<Vector> = sub( v, 4UL, 8UL );
   \endcode
*/
template< typename VT  // Type of the vector
        , bool TF >    // Transpose flag
inline typename DisableIf< Or< IsComputation<VT>, IsTransExpr<VT> >, SparseSubvector<const VT> >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return SparseSubvector<const VT>( ~sv, start, n );
}
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL RESTRUCTURING OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector addition.
// \ingroup views
//
// \param sv The constant vector/vector addition.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the addition.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector addition.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename EnableIf< IsVecVecAddExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return sub( (~sv).leftOperand(), start, n ) + sub( (~sv).rightOperand(), start, n );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector subtraction.
// \ingroup views
//
// \param sv The constant vector/vector subtraction.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the subtraction.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector subtraction.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename EnableIf< IsVecVecSubExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return sub( (~sv).leftOperand(), start, n ) - sub( (~sv).rightOperand(), start, n );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector multiplication.
// \ingroup views
//
// \param sv The constant vector/vector multiplication.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the multiplication.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector multiplication.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Transpose flag
inline typename EnableIf< IsVecVecMultExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return sub( (~sv).leftOperand(), start, n ) * sub( (~sv).rightOperand(), start, n );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/scalar multiplication.
// \ingroup views
//
// \param sv The constant vector/scalar multiplication.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the multiplication.
//
// This function returns an expression representing the specified subvector of the given
// vector/scalar multiplication.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Storage order
inline typename EnableIf< IsVecScalarMultExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return sub( (~sv).leftOperand(), start, n ) * (~sv).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/scalar division.
// \ingroup views
//
// \param sv The constant vector/scalar division.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the division.
//
// This function returns an expression representing the specified subvector of the given
// vector/scalar division.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Storage order
inline typename EnableIf< IsVecScalarDivExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return sub( (~sv).leftOperand(), start, n ) / (~sv).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector abs operation.
// \ingroup views
//
// \param sv The constant vector abs operation.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the abs operation.
//
// This function returns an expression representing the specified subvector of the given vector
// abs operation.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Storage order
inline typename EnableIf< IsVecAbsExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return abs( sub( (~sv).operand(), start, n ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector evaluation operation.
// \ingroup views
//
// \param sv The constant vector evaluation operation.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the evaluation operation.
//
// This function returns an expression representing the specified subvector of the given vector
// evaluation operation.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Storage order
inline typename EnableIf< IsVecEvalExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return eval( sub( (~sv).operand(), start, n ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector transpose operation.
// \ingroup views
//
// \param sv The constant vector transpose operation.
// \param start The index of the first element of the subvector.
// \param n The size of the subvector.
// \return View on the specified subvector of the transpose operation.
//
// This function returns an expression representing the specified subvector of the given vector
// transpose operation.
*/
template< typename VT  // Type of the sparse vector
        , bool TF >    // Storage order
inline typename EnableIf< IsVecTransExpr<VT>, typename SubvectorExprTrait<VT>::Type >::Type
   sub( const SparseVector<VT,TF>& sv, size_t start, size_t n )
{
   BLAZE_FUNCTION_TRACE;

   return trans( sub( (~sv).operand(), start, n ) );
}
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif