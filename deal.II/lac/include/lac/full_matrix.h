/*----------------------------   fullmatrix.h     ---------------------------*/
//      $Id$
#ifndef __lac_fullmatrix_H
#define __lac_fullmatrix_H
/*----------------------------   fullmatrix.h     ---------------------------*/

// This file is part of the DEAL Library
// DEAL is Copyright(1995) by
// Roland Becker, Guido Kanschat, Franz-Theo Suttmeier
// Revised by Wolfgang Bangerth


#include <base/exceptions.h>
#include <base/subscriptor.h>


// forward declarations

template<typename number> class Vector;

class iVector;



/**
 * Rectangular/quadratic full matrix.
 *
 * Implementation of a classical rectangular scheme of numbers. The
 * data type of the entries is provided in the template argument
 * #number#.  The interface is quite fat and in fact has grown every
 * time a new feature was needed. So, a lot of functions are provided.
 *
 * Since the instantiation of this template is quite an effort,
 * standard versions are precompiled into the library. These include
 * all combinations of #float# and #double# for matrices and
 * vectors. If you need more data types, the implementation of
 * non-inline functions is in #fullmatrix.templates.h#. Driver files
 * are in the source tree.
 *
 * Internal calculations are usually done with the accuracy of the
 * vector argument to functions. If there is no argument with a number
 * type, the matrix number type is used.
 *
 * <TABLE BORDER=1>
 * <TR><TH ALIGN=CENTER><B>this</B><TH ALIGN=CENTER><B>other
 * matrix</B><TH ALIGN=CENTER><B>vector</B><TH ALIGN=CENTER><B>rhs in
 * residual</B></TR>
 * <TR><TD ALIGN=CENTER>double<TD ALIGN=CENTER>double<TD ALIGN=CENTER>double<TD ALIGN=CENTER>double</TR>
 * <TR><TD ALIGN=CENTER>double<TD ALIGN=CENTER>double<TD ALIGN=CENTER>float<TD ALIGN=CENTER>double</TR>
 * <TR><TD ALIGN=CENTER>double<TD ALIGN=CENTER>double<TD ALIGN=CENTER>float<TD ALIGN=CENTER>float</TR>
 * <TR><TD ALIGN=CENTER>float<TD ALIGN=CENTER>float<TD ALIGN=CENTER>double<TD ALIGN=CENTER>double</TR>
 * <TR><TD ALIGN=CENTER>float<TD ALIGN=CENTER>float<TD ALIGN=CENTER>float<TD ALIGN=CENTER>double</TR>
 * <TR><TD ALIGN=CENTER>float<TD ALIGN=CENTER>float<TD ALIGN=CENTER>float<TD ALIGN=CENTER>float</TR>
 * <CAPTION>Instantiations provided in the library</CAPTION>
 * </TABLE>
 *
 * In the documentation of member functions, the following conventions are adopted:
 * \begin{itemize}
 *  \item THIS matrix is always named #A#.
 *  \item Matrices are always uppercase, vectors and scalars are lowercase.
 *  \item #Transp(A)# denotes the transpose of matrix A.
 * \end{itemize}
 *
 * @author Guido Kanschat, Franz-Theo Suttmeier, Wolfgang Bangerth
 */
template<typename number>
class FullMatrix : public Subscriptor
{
  public:
				     /**
				      * Constructor. Initialize the matrix as
				      * a square matrix with dimension #n#.
				      *
				      * In order to avoid the implicit
				      * conversion of integers and other types
				      * to a matrix, this constructor is
				      * declared #explicit#.
				      */
    explicit FullMatrix (const unsigned int n = 1);
    
				     /**
				      * Constructor. Initialize the matrix as
				      * a rectangular matrix.
				      */
    FullMatrix (const unsigned int rows, const unsigned int cols);
    
				     /** 
				      * Copy constructor. Be very careful with
				      * this constructor, since it may take a
				      * huge amount of computing time for large
				      * matrices!!
				      */
    explicit FullMatrix (const FullMatrix&);

				     /**
				      * Destructor. Release all memory.
				      */
    ~FullMatrix();
    
				     /**
				      * Comparison operator. Be careful with
				      * this thing, it may eat up huge amounts
				      * of computing time! It is most commonly
				      * used for internal consistency checks
				      * of programs.
				      */
    bool operator == (const FullMatrix<number> &) const;

				     /**
				      * Assignment operator.
				      * Copy all elements of #src#
				      * into the matrix. The size is
				      * adjusted if needed.
				      *
				      * We can't use the other, templatized
				      * version since if we don't declare
				      * this one, the compiler will happily
				      * generate a predefined copy
				      * operator which is not what we want.
				      */
    FullMatrix<number>& operator = (const FullMatrix<number>& src);

				     /**
				      * Assignment operator.
				      * Copy all elements of #src#
				      * into the matrix. The size is
				      * adjusted if needed.
				      */
    template<typename number2>
    FullMatrix<number>& operator = (const FullMatrix<number2>& src);
    
    
				     /**
				      * Fill rectangular block.
				      *
				      * The matrix #src# is copied
				      * into the target. The optional
				      * values #i# and #j# determine the
				      * upper left corner of the image
				      * of #src#.
				      *
				      * This function requires that
				      * #i+src.m()<=m()# and
				      * #j+src.n()<=n()#, that is, the
				      * image fits into the space of #this#.
				      */
    template<typename number2>
    void fill (const FullMatrix<number2> &src,
	       const unsigned int         i=0,
	       const unsigned int         j=0);
    
				     /**
				      * Set dimension to $m\times n$ and
				      * allocate memory if necessary. Forget
				      * the previous content of the matrix.
				      */
    void reinit (const unsigned int m,
		 const unsigned int n);
    
				     /**
				      * Set dimension to $n\times n$ and
				      * allocate memory if necessary. Forget
				      * the previous content of the matrix.
				      */
    void reinit (const unsigned int n);
    
				     /**
				      * Set dimension to $m(B)\times n(B)$ and
				      * allocate memory if necessary. Forget
				      * the previous content of the matrix.
				      */
    template<typename number2>
    void reinit (const FullMatrix<number2> &B);
    
				     /**
				      * Number of rows of this matrix.
				      * To remember: this matrix is an
				      * $m \times n$-matrix.
				      */
    unsigned int m () const;
    
				     /**
				      * Number of columns of this matrix.
				      * To remember: this matrix is an
				      * $m \times n$-matrix.
				      */
    unsigned int n () const;

    				     /**
				      * Return whether the matrix contains only
				      * elements with value zero. This function
				      * is mainly for internal consistency
				      * checks and should seldomly be used when
				      * not in debug mode since it uses quite
				      * some time.
				      */
    bool all_zero () const;

				     /**
				      * Return the value of the element #(i,j)#.
				      * Does the same as the private #el(i,j)#
				      * function but does bounds checking in
				      * debug mode.
				      */
    number operator() (const unsigned int i,
		       const unsigned int j) const;
    
				     /**
				      * Return a read-write reference to
				      * the element #(i,j)#.
				      * Does the same as the private #el(i,j)#
				      * function but does bounds checking in
				      * debug mode.
				      */
    number& operator() (const unsigned int i,
			const unsigned int j);
    
				     /**
				      * Set all entries in the matrix to
				      * zero. Do not resize the matrix.
				      */
    void clear ();

				     /**
				      * Weighted addition. The matrix
				      * #s*B# is added to #this#.
				      *
				      * $A += sB$
				      */
    template<typename number2>
    void add (const number               s,
	      const FullMatrix<number2> &B);

				     /**
				      * Weighted addition of the
				      * transpose of #B# to #this#.
				      *
				      * $A += s B^T$
				      */
    template<typename number2>
    void Tadd (const number               s,
	       const FullMatrix<number2> &B);
    
				     /**
				      * Matrix-matrix-multiplication.
				      * $C=A*B$.
				      */
    template<typename number2>
    void mmult (FullMatrix<number2>       &C,
		const FullMatrix<number2> &B) const;
    
				     /**
				      * Matrix-matrix-multiplication using
				      * transpose of #this#.
				      * $C=A^T*B$.
				      */
    template<typename number2>
    void Tmmult (FullMatrix<number2>       &C,
		 const FullMatrix<number2> &B) const;
    
				     /**
				      * Matrix-vector-multiplication.
				      *
				      * The optional parameter
				      * #adding# determines, whether the
				      * result is stored in #w# or added
				      * to #w#.
				      *
				      * if (adding)
				      *  $w += A*v$
				      *
				      * if (!adding)
				      *  $w = A*v$
				      */
    template<typename number2>
    void vmult (Vector<number2>       &w,
		const Vector<number2> &v,
		const bool             adding=false) const;
    
				     /**
				      * Transpose matrix-vector-multiplication.
				      * See #vmult# above.
				      */
    template<typename number2>
    void Tvmult (Vector<number2>       &w,
		 const Vector<number2> &v,
		 const bool             adding=false) const;

				     /**
				      * Return the norm of the vector #v# with
				      * respect to the norm induced by this
				      * matrix, i.e. $\left(v,Mv\right)$. This
				      * is useful, e.g. in the finite element
				      * context, where the $L_2$ norm of a
				      * function equals the matrix norm with
				      * respect to the mass matrix of the vector
				      * representing the nodal values of the
				      * finite element function.
				      *
				      * Note the order in which the matrix
				      * appears. For non-symmetric matrices
				      * there is a difference whether the
				      * matrix operates on the first
				      * or on the second operand of the
				      * scalar product.
				      *
				      * Obviously, the matrix needs to be square
				      * for this operation.
				      */
    template<typename number2>
    double matrix_norm (const Vector<number2> &v) const;

				     /**
				      * Build the matrix scalar product
				      * #u^T M v#. This function is mostly
				      * useful when building the cellwise
				      * scalar product of two functions in
				      * the finite element context.
				      */
    template<typename number2>
    double matrix_scalar_product (const Vector<number2> &u,
				  const Vector<number2> &v) const;

    				     /**
				      * Return the $l_1$-norm of the matrix, i.e.
				      * $|M|_1=max_{all columns j}\sum_{all 
				      * rows i} |M_ij|$,
				      * (max. sum of columns). This is the
				      * natural matrix norm that is compatible
				      * to the $l_1$-norm for vectors, i.e.
				      * $|Mv|_1\leq |M|_1 |v|_1$.
				      * (cf. Rannacher Numerik0)
				      */
    number l1_norm () const;

    				     /**
				      * Return the $l_\infty$-norm of the
				      * matrix, i.e.
				      * $|M|_\infty=\max_{all rows i}\sum_{all 
				      * columns j} |M_{ij}|$,
				      * (max. sum of rows).
				      * This is the
				      * natural matrix norm that is compatible
				      * to the $l_\infty$-norm of vectors, i.e.
				      * $|Mv|_\infty \leq |M|_\infty |v|_\infty$.
				      * (cf. Rannacher Numerik0)
				      */
    number linfty_norm () const;
    
				     /**
				      * Compute the quadratic matrix norm.
				      * Return value is the root of the square
				      * sum of all matrix entries. Also called
				      * Frobenius Norm.
				      * 
				      * This norm is compatible with the l2
				      * vector norm. But it is not a natural
				      * matrix norm (cf Rannacher Numeric0),
				      * therefore it is not called l2_norm.
				      */
    number norm2 () const;
    
				     /**
				      * A=Inverse(A). Inversion of this by
				      * Gauss-Jordan-algorithm. Note that this
				      * is a rather expensive operation, so
				      * you may not want to use it for
				      * larger matrices if not necessary.
				      */
    void gauss_jordan ();

				     /**
                                      * Computes the determinant of a matrix.
                                      * This is only implemented for one two and
                                      * three dimensions, since for higher
                                      * dimensions the numerical work explodes.
                                      * Obviously, the matrix needs to be square
                                      * for this function.
                                      */
    double determinant () const;

				     /**
				      * Assign the inverse of the given
				      * matrix to #*this#. This function is
				      * only implemented (hardcoded) for
				      * square matrices of dimension one,
				      * two, three and four, since the
				      * amount of code needed grows quickly.
				      * The implementation does not use
				      * an elimination method like the
				      * Gauss-Jordan one, but rather sets
				      * the element directly; their values
				      * are precomputed symbolically using
				      * Maple. This way, we can avoid the
				      * overhead of loops and local variables
				      * but the number of lines of code
				      * grows rapidly.
				      *
				      * For all other sizes than the ones given
				      * above, an exception of type
				      * #ExcNotImplemented(dim_range)# is
				      * thrown, which you can catch and use
				      * some other method to invert the matrix,
				      * e.g. the #gauss_jordan# function.
				      */
    void invert (const FullMatrix<number> &M);

				     /**
				      * $A(i,1-n)+=s*A(j,1-n)$.
				      * Simple addition of rows of this
				      */
    void add_row (const unsigned int i,
		  const number       s,
		  const unsigned int j);

				     /**
				      * $A(i,1-n)+=s*A(j,1-n)+t*A(k,1-n)$.
				      * Multiple addition of rows of this.
				      */
    void add_row (const unsigned int i,
		  const number s, const unsigned int j,
		  const number t, const unsigned int k);

				     /**
				      * $A(1-n,i)+=s*A(1-n,j)$.
				      *  Simple addition of columns of this.
				      */
    void add_col (const unsigned int i,
		  const number       s,
		  const unsigned int j);

				     /**
				      * $A(1-n,i)+=s*A(1-n,j)+t*A(1-n,k)$.
				      *  Multiple addition of columns of this.
				      */
    void add_col (const unsigned int i,
		  const number s, const unsigned int j,
		  const number t, const unsigned int k);

				     /**
				      * Swap  A(i,1-n) <-> A(j,1-n).
				      * Swap rows i and j of this
				      */
    void swap_row (const unsigned int i, const unsigned int j);

				     /**
				      *  Swap  A(1-n,i) <-> A(1-n,j).
				      *  Swap columns i and j of this
				      */
    void swap_col (const unsigned int i, const unsigned int j);

				     /**
				      * $w=b-A*v$.
				      * Residual calculation , returns
				      * the $l_2$-norm $|w|$
				      */
    template<typename number2, typename number3>
    double residual (Vector<number2>      & w,
		     const Vector<number2>& v,
		     const Vector<number3>& b) const;

				     /**
				      * Forward elimination of lower triangle.
				      * Inverts the lower triangle of a
				      * quadratic matrix.
				      *
				      * If the matrix has more columns than rows,
				      * this function only operates on the left
				      * square submatrix. If there are more rows,
				      * the upper square part of the matrix
				      * is considered
				      */
    template<typename number2>
    void forward (Vector<number2>       &dst,
		  const Vector<number2> &src) const;

				     /**
				      * Backward elimination of upper triangle.
				      * @see forward
				      */
    template<typename number2>
    void backward (Vector<number2>       &dst,
		   const Vector<number2> &src) const;

				     /**
				      * QR-factorization of a matrix.
				      * The orthogonal transformation Q is
				      * applied to the vector y and this matrix.
				      *
				      * After execution of householder, the upper
				      * triangle contains the resulting matrix R,
				      * the lower the incomplete factorization
				      * matrices.
				      */
    template<typename number2>
    void householder (Vector<number2> &y);

				     /**
				      * Least-Squares-Approximation by
				      * QR-factorization.
				      */
    template<typename number2>
    double least_squares (Vector<number2> &dst,
			  Vector<number2> &src);

				     /**
				      *  A(i,i)+=B(i,1-n). Addition of complete
				      *  rows of B to diagonal-elements of this ; <p>
				      *  ( i = 1 ... m )
				      */
    template<typename number2>
    void add_diag (const number               s,
		   const FullMatrix<number2> &B);

				     /**
				      *  A(i,i)+=s  i=1-m.
				      * Add constant to diagonal elements of this
				      */
    void diagadd (const number s);

				     /**
				      *  w+=part(A)*v. Conditional partial
				      *  Matrix-vector-multiplication <p>
				      *  (used elements of v determined by x)
				      */
    template<typename number2>
    void gsmult (Vector<number2>& w, const Vector<number2>& v, const iVector& x) const;


				     /**
				      * Output of the matrix in user-defined format.
				      */
    void print (ostream& s, int width=5, int precision=2) const;

				     /**
				      * Print the matrix in the usual format,
				      * i.e. as a matrix and not as a list of
				      * nonzero elements. For better
				      * readability, zero elements
				      * are displayed as empty space.
				      *
				      * Each entry is printed in scientific
				      * format, with one pre-comma digit and
				      * the number of digits given by
				      * #precision# after the comma, with one
				      * space following.
				      * The precision defaults to four, which
				      * suffices for most cases. The precision
				      * and output format are {\it not}
				      * properly reset to the old values
				      * when the function exits.
				      *
				      * You should be aware that this function
				      * may produce {\bf large} amounts of
				      * output if applied to a large matrix!
				      * Be careful with it.
				      */
    void print_formatted (ostream &out,
			  const unsigned int presicion=3) const;
				     //@}

				     /**
				      * Exception
				      */
    DeclException2 (ExcInvalidIndex,
		    int, int,
		    << "The given index " << arg1
		    << " should be less than " << arg2 << ".");
				     /**
				      * Exception
				      */
    DeclException2 (ExcDimensionMismatch,
		    int, int,
		    << "The two dimensions " << arg1 << " and " << arg2
		    << " do not match here.");
				     /**
				      * Exception
				      */
    DeclException0 (ExcNotQuadratic);
				     /**
				      * Exception
				      */
    DeclException0 (ExcNotRegular);
				     /**
				      * Exception
				      */
    DeclException3 (ExcInvalidDestination,
		    int, int, int,
		    << "Target region not in matrix: size in this direction="
		    << arg1 << ", size of new matrix=" << arg2
		    << ", offset=" << arg3);
				     /**
				      * Exception
				      */
    DeclException1 (ExcNotImplemented,
		    int,
		    << "This function is not implemented for the given"
		    << " matrix dimension " << arg1);
				     /**
				      * Exception
				      */
    DeclException0 (ExcIO);

  private:
				     /**
				      * Component-array.
				      */
    number *val;
    
				     /** 
				      * Dimension of range.
				      * Actual number of Columns
				      */
    unsigned int dim_range;
    
				     /**
				      * Dimension of image. Actual number of Rows
				      */
    unsigned int dim_image;
    
				     /**
				      * Dimension of the array
				      * holding the values of the
				      * matrix elements. Determines
				      * amount of reserved memory.
				      *
				      * Actually, the allocated array may
				      * not have a size equal to the number
				      * of elements of this matrix, since
				      * reallocation only happens when the
				      * size of the matrix is increased.
				      * Therefore, if the matrix size was
				      * decreased somewhen in the past,
				      * #val_size# will be larger than
				      * #dim_range * dim_image#.
				      */
    unsigned int val_size;
    
				     /**
				      * Initialization. Initialize
				      * memory for a #FullMatrix#
				      * of #m# rows and #n#
				      * columns to zero.
				      */
    void init (const unsigned int m, const unsigned int n);
    
				     /**
				      * Return a read-write reference to the
				      * element #(i,j)#.
				      *
				      * This function does no bounds
				      * checking and is only to be used
				      * internally and in functions
				      * already checked.
				      */
    number & el (const unsigned int i, const unsigned int j);
    
				     /**
				      * Return the value of the element #(i,j)#.
				      *
				      * This function does no bounds checking
				      * and is only to be used
				      * internally and in functions
				      * already checked.
				      */
    number el (const unsigned int i, const unsigned int j) const;    
};





/*-------------------------Inline functions -------------------------------*/

template <typename number>
inline number &
FullMatrix<number>::el (const unsigned int i, const unsigned int j)
{
  return val[i*dim_range+j];
};


template <typename number>
inline number
FullMatrix<number>::el (const unsigned int i, const unsigned int j) const
{
  return val[i*dim_range+j];
};


template <typename number>
inline unsigned int
FullMatrix<number>::m() const
{
  return dim_image;
};


template <typename number>
inline unsigned int
FullMatrix<number>::n() const
{
  return dim_range;
};


// template <typename number>
// inline number
// FullMatrix<number>::el (const unsigned int i) const
// {
//   return val[i];
// };


template <typename number>
inline number
FullMatrix<number>::operator() (const unsigned int i, const unsigned int j) const
{  
  Assert (i<dim_image, ExcInvalidIndex (i, dim_image));
  Assert (j<dim_range, ExcInvalidIndex (i, dim_range));
  return el(i,j);
};


template <typename number>
inline number &
FullMatrix<number>::operator() (const unsigned int i, const unsigned int j)
{
  Assert (i<dim_image, ExcInvalidIndex (i, dim_image));
  Assert (j<dim_range, ExcInvalidIndex (j, dim_range));
  return el(i,j);
}




/*----------------------------   fullmatrix.h     ---------------------------*/
/* end of #ifndef __lac_fullmatrix_H */
#endif
/*----------------------------   fullmatrix.h     ---------------------------*/
