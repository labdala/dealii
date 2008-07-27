//---------------------------------------------------------------------------
//    $Id: chunk_sparsity_pattern.cc 14783 2007-06-18 14:52:01Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <lac/chunk_sparsity_pattern.h>
#include <lac/compressed_sparsity_pattern.h>
#include <lac/compressed_set_sparsity_pattern.h>
#include <lac/full_matrix.h>


DEAL_II_NAMESPACE_OPEN


ChunkSparsityPattern::ChunkSparsityPattern ()
{
  reinit (0,0,0,0);
}



ChunkSparsityPattern::ChunkSparsityPattern (const ChunkSparsityPattern &s)
                :
		Subscriptor(),
		chunk_size (s.chunk_size),
		sparsity_pattern(s.sparsity_pattern)
{
  Assert (s.rows == 0, ExcInvalidConstructorCall());
  Assert (s.cols == 0, ExcInvalidConstructorCall());
  
  reinit (0,0,0,0, false);
}



ChunkSparsityPattern::ChunkSparsityPattern (const unsigned int m,
					    const unsigned int n,
					    const unsigned int max_per_row,
					    const unsigned int chunk_size,
					    const bool optimize_diag)
{
  reinit (m,n,max_per_row, chunk_size, optimize_diag);
}



ChunkSparsityPattern::ChunkSparsityPattern (
  const unsigned int m,
  const unsigned int n,
  const std::vector<unsigned int>& row_lengths,
  const unsigned int chunk_size,
  const bool optimize_diag) 
{
  reinit (m, n, row_lengths, chunk_size, optimize_diag);
}



ChunkSparsityPattern::ChunkSparsityPattern (const unsigned int n,
					    const unsigned int max_per_row,
					    const unsigned int chunk_size)
{
  reinit (n, n, max_per_row, chunk_size, true);
}



ChunkSparsityPattern::ChunkSparsityPattern (
  const unsigned int               m,
  const std::vector<unsigned int>& row_lengths,
  const unsigned int chunk_size,
  const bool optimize_diag) 
{
  reinit (m, m, row_lengths, chunk_size, optimize_diag);
}



ChunkSparsityPattern::~ChunkSparsityPattern ()
{}



ChunkSparsityPattern &
ChunkSparsityPattern::operator = (const ChunkSparsityPattern &s)
{
  Assert (s.rows == 0, ExcInvalidConstructorCall());
  Assert (s.cols == 0, ExcInvalidConstructorCall());

				   // perform the checks in the underlying
				   // object as well
  sparsity_pattern = s.sparsity_pattern;
  
  return *this;
}



void
ChunkSparsityPattern::reinit (const unsigned int m,
			      const unsigned int n,
			      const unsigned int max_per_row,
			      const unsigned int chunk_size,
			      const bool optimize_diag)
{
				   // simply map this function to the
				   // other @p{reinit} function
  const std::vector<unsigned int> row_lengths (m, max_per_row);
  reinit (m, n, row_lengths, chunk_size, optimize_diag);
}



void
ChunkSparsityPattern::reinit (
  const unsigned int m,
  const unsigned int n,
  const VectorSlice<const std::vector<unsigned int> >&row_lengths,
  const unsigned int chunk_size,
  const bool optimize_diag)
{
  Assert (row_lengths.size() == m, ExcInvalidNumber (m));
	  
  rows = m;
  cols = n;

  this->chunk_size = chunk_size;

				   // pass down to the necessary information
				   // to the underlying object. we need to
				   // calculate how many chunks we need: we
				   // need to round up (m/chunk_size) and
				   // (n/chunk_size). rounding up in integer
				   // arithmetic equals
				   // ((m+chunk_size-1)/chunk_size):
  const unsigned int m_chunks = (m+chunk_size) / chunk_size,
		     n_chunks = (n+chunk_size) / chunk_size;  

				   // compute the maximum number of chunks in
				   // each row. the passed array denotes the
				   // number of entries in each row -- in the
				   // worst case, these are all in independent
				   // chunks, so we have to calculate it as
				   // follows:
  std::vector<unsigned int> chunk_row_lengths (m_chunks, 0);
  for (unsigned int i=0; i<m; ++i)
    chunk_row_lengths[i/chunk_size]
      = std::max (chunk_row_lengths[i/chunk_size],
		  row_lengths[i]);

  sparsity_pattern.reinit (m_chunks,
			   n_chunks,
			   chunk_row_lengths,
			   optimize_diag);
}



void
ChunkSparsityPattern::compress ()
{
  sparsity_pattern.compress ();
}



void
ChunkSparsityPattern::copy_from (const CompressedSparsityPattern &csp,
				 const unsigned int chunk_size,
				 const bool         optimize_diag) 
{
				   // count number of entries per row, then
				   // initialize the underlying sparsity
				   // pattern
  std::vector<unsigned int> entries_per_row (csp.n_rows(), 0);
  for (unsigned int row = 0; row<csp.n_rows(); ++row)
    for (unsigned int j=0; j<csp.row_length(row); ++j)
      ++entries_per_row[row];

  reinit (csp.n_rows(), csp.n_cols(),
	  entries_per_row,
	  chunk_size, optimize_diag);
      
				   // then actually fill it  
  for (unsigned int row = 0; row<csp.n_rows(); ++row)
    for (unsigned int j=0; j<csp.row_length(row); ++j)
      add (row, csp.column_number(row,j));

				   // finally compress
  compress ();
}



void
ChunkSparsityPattern::copy_from (const CompressedSetSparsityPattern &csp,
				 const unsigned int chunk_size,
				 const bool         optimize_diag) 
{
				   // count number of entries per row, then
				   // initialize the underlying sparsity
				   // pattern
  std::vector<unsigned int> entries_per_row (csp.n_rows(), 0);
  for (unsigned int row = 0; row<csp.n_rows(); ++row)
    {
      CompressedSetSparsityPattern::row_iterator col_num = csp.row_begin (row);

      for (; col_num != csp.row_end (row); ++col_num)
	++entries_per_row[row];
    }

  reinit (csp.n_rows(), csp.n_cols(),
	  entries_per_row,
	  chunk_size, optimize_diag);
      
				   // then actually fill it  
  for (unsigned int row = 0; row<csp.n_rows(); ++row)
    {
      CompressedSetSparsityPattern::row_iterator col_num = csp.row_begin (row);

      for (; col_num != csp.row_end (row); ++col_num)
	add (row, *col_num);
    }
  
				   // finally compress
  compress ();
}




template <typename number>
void ChunkSparsityPattern::copy_from (const FullMatrix<number> &matrix,
				      const unsigned int chunk_size,
				      const bool         optimize_diag)
{
				   // count number of entries per row, then
				   // initialize the underlying sparsity
				   // pattern
  std::vector<unsigned int> entries_per_row (matrix.m(), 0);
  for (unsigned int row=0; row<matrix.m(); ++row)
    for (unsigned int col=0; col<matrix.n(); ++col)
      if (matrix(row,col) != 0)
	++entries_per_row[row];

  reinit (matrix.m(), matrix.n(),
	  entries_per_row,
	  chunk_size, optimize_diag);
      
				   // then actually fill it  
  for (unsigned int row=0; row<matrix.m(); ++row)
    for (unsigned int col=0; col<matrix.n(); ++col)
      if (matrix(row,col) != 0)
	add (row,col);

				   // finally compress
  compress ();
}


void
ChunkSparsityPattern::reinit (
  const unsigned int m,
  const unsigned int n,
  const std::vector<unsigned int>& row_lengths,
  const unsigned int chunk_size,
  const bool optimize_diag)
{
  reinit(m, n, make_slice(row_lengths), chunk_size, optimize_diag);
}



bool
ChunkSparsityPattern::empty () const
{
  return sparsity_pattern.empty();
}



unsigned int
ChunkSparsityPattern::max_entries_per_row () const 
{
  return sparsity_pattern.max_entries_per_row() * chunk_size;
}



void
ChunkSparsityPattern::add (const unsigned int i,
			   const unsigned int j)
{
  Assert (i<rows, ExcInvalidIndex(i,rows));
  Assert (j<cols, ExcInvalidIndex(j,cols));

  sparsity_pattern.add (i/chunk_size, j/chunk_size);
}


bool
ChunkSparsityPattern::exists (const unsigned int i,
			      const unsigned int j) const
{
  Assert (i<rows, ExcIndexRange(i,0,rows));
  Assert (j<cols, ExcIndexRange(j,0,cols));

  return sparsity_pattern.exists (i/chunk_size,
				  j/chunk_size);
}



void
ChunkSparsityPattern::symmetrize () 
{
				   // matrix must be square. note that the for
				   // some matrix sizes, the current sparsity
				   // pattern may not be square even if the
				   // underlying sparsity pattern is (e.g. a
				   // 10x11 matrix with chunk_size 4)
  Assert (rows==cols, ExcNotQuadratic());

  sparsity_pattern.symmetrize ();
}



void
ChunkSparsityPattern::print (std::ostream &/*out*/) const
{
  Assert (false, ExcNotImplemented());
}



void
ChunkSparsityPattern::print_gnuplot (std::ostream &/*out*/) const
{
  Assert (false, ExcNotImplemented());
}



unsigned int
ChunkSparsityPattern::bandwidth () const
{
				   // calculate the bandwidth from that of the
				   // underlying sparsity pattern. note that
				   // even if the bandwidth of that is zero,
				   // then the bandwidth of the chunky pattern
				   // is chunk_size-1, if it is 1 then the
				   // chunky pattern has
				   // chunk_size+(chunk_size-1), etc
				   //
				   // we'll cut it off at max(n(),m())
  return std::min (sparsity_pattern.bandwidth()*chunk_size
		   + (chunk_size-1),
		   std::max(n_rows(), n_cols()));
}


void
ChunkSparsityPattern::block_write (std::ostream &out) const 
{
  AssertThrow (out, ExcIO());

                                   // first the simple objects,
                                   // bracketed in [...]
  out << '['
      << rows << ' '
      << cols << ' '
      << "][";
				   // then the underlying sparsity pattern
  sparsity_pattern.block_write (out);
  out << ']';
  
  AssertThrow (out, ExcIO());  
}



void
ChunkSparsityPattern::block_read (std::istream &in)
{
  AssertThrow (in, ExcIO());

  char c;

                                   // first read in simple data
  in >> c;
  AssertThrow (c == '[', ExcIO());
  in >> rows
     >> cols;

  in >> c;
  AssertThrow (c == ']', ExcIO());
  in >> c;
  AssertThrow (c == '[', ExcIO());

				   // then read the underlying sparsity
				   // pattern
  sparsity_pattern.block_read (in);

  in >> c;
  AssertThrow (c == ']', ExcIO());  
}



unsigned int
ChunkSparsityPattern::memory_consumption () const
{
  return (sizeof(*this) +
	  sparsity_pattern.memory_consumption());
}



// explicit instantiations
template
void ChunkSparsityPattern::copy_from<float> (const FullMatrix<float> &,
					     const unsigned int,
					     bool);
template
void ChunkSparsityPattern::copy_from<double> (const FullMatrix<double> &,
					      const unsigned int,
					      bool);

DEAL_II_NAMESPACE_CLOSE
