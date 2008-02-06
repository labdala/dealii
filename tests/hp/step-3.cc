//----------------------------  step-3.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2006, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  step-3.cc  ---------------------------


// a hp-ified version of step-3


#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <dofs/dof_handler.h>
#include <grid/grid_generator.h>

#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>

#include <fe/fe_q.h>

#include <dofs/dof_tools.h>

#include <hp/fe_values.h>
#include <base/quadrature_lib.h>

#include <base/function.h>
#include <numerics/vectors.h>
#include <numerics/matrices.h>

#include <lac/vector.h>
#include <lac/full_matrix.h>
#include <lac/sparse_matrix.h>
#include <lac/solver_cg.h>
#include <lac/precondition.h>

#include <numerics/data_out.h>
#include <fstream>
#include <iostream>

std::ofstream logfile("step-3/output");




class LaplaceProblem 
{
  public:
    LaplaceProblem ();

    void run ();
    
  private:
    void make_grid_and_dofs ();
    void assemble_system ();
    void solve ();
    void output_results () const;

    Triangulation<2>     triangulation;
    hp::FECollection<2>              fe;
    hp::DoFHandler<2>        dof_handler;

    SparsityPattern      sparsity_pattern;
    SparseMatrix<double> system_matrix;

    Vector<double>       solution;
    Vector<double>       system_rhs;
};


LaplaceProblem::LaplaceProblem () :
                fe (FE_Q<2>(1)),
		dof_handler (triangulation)
{}



void LaplaceProblem::make_grid_and_dofs ()
{
  GridGenerator::hyper_cube (triangulation, -1, 1);
  triangulation.refine_global (5);
  deallog << "Number of active cells: "
	    << triangulation.n_active_cells()
	    << std::endl;
  deallog << "Total number of cells: "
	    << triangulation.n_cells()
	    << std::endl;
  
  dof_handler.distribute_dofs (fe);
  deallog << "Number of degrees of freedom: "
	    << dof_handler.n_dofs()
	    << std::endl;

  sparsity_pattern.reinit (dof_handler.n_dofs(),
			   dof_handler.n_dofs(),
			   dof_handler.max_couplings_between_dofs());
  DoFTools::make_sparsity_pattern (dof_handler, sparsity_pattern);
  sparsity_pattern.compress();

  system_matrix.reinit (sparsity_pattern);

  solution.reinit (dof_handler.n_dofs());
  system_rhs.reinit (dof_handler.n_dofs());
}



void LaplaceProblem::assemble_system () 
{
  hp::QCollection<2>  quadrature_formula(QGauss<2>(2));
  hp::FEValues<2> x_fe_values (fe, quadrature_formula, 
			 update_values | update_gradients | update_JxW_values);
  
  const unsigned int   dofs_per_cell = fe[0].dofs_per_cell;
  const unsigned int   n_q_points    = quadrature_formula[0].n_quadrature_points;

  FullMatrix<double>   cell_matrix (dofs_per_cell, dofs_per_cell);
  Vector<double>       cell_rhs (dofs_per_cell);

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

  hp::DoFHandler<2>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    {
      x_fe_values.reinit (cell);

      const FEValues<2> &fe_values = x_fe_values.get_present_fe_values();
      
      cell_matrix = 0;
      cell_rhs = 0;

      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int j=0; j<dofs_per_cell; ++j)
	  for (unsigned int q_point=0; q_point<n_q_points; ++q_point)
	    cell_matrix(i,j) += (fe_values.shape_grad (i, q_point) *
				 fe_values.shape_grad (j, q_point) *
				 fe_values.JxW (q_point));

      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int q_point=0; q_point<n_q_points; ++q_point)
	  cell_rhs(i) += (fe_values.shape_value (i, q_point) *
			  1 *
			  fe_values.JxW (q_point));

      cell->get_dof_indices (local_dof_indices);

      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int j=0; j<dofs_per_cell; ++j)
	  system_matrix.add (local_dof_indices[i],
			     local_dof_indices[j],
			     cell_matrix(i,j));

      for (unsigned int i=0; i<dofs_per_cell; ++i)
	system_rhs(local_dof_indices[i]) += cell_rhs(i);
    }


  std::map<unsigned int,double> boundary_values;
  VectorTools::interpolate_boundary_values (dof_handler,
					    0,
					    ZeroFunction<2>(),
					    boundary_values);
  MatrixTools::apply_boundary_values (boundary_values,
				      system_matrix,
				      solution,
				      system_rhs);
}



void LaplaceProblem::solve () 
{
  SolverControl           solver_control (1000, 1e-12);
  SolverCG<>              cg (solver_control);

  cg.solve (system_matrix, solution, system_rhs,
	    PreconditionIdentity());
}



void LaplaceProblem::output_results () const
{
  DataOut<2,hp::DoFHandler<2> > data_out;
  data_out.attach_dof_handler (dof_handler);
  data_out.add_data_vector (solution, "solution");
  data_out.build_patches ();

  data_out.write_gnuplot (deallog.get_file_stream());
}



void LaplaceProblem::run () 
{
  make_grid_and_dofs ();
  assemble_system ();
  solve ();
  output_results ();
}



int main () 
{
  logfile.precision(2);
  deallog << std::setprecision(2);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);  

  LaplaceProblem laplace_problem;
  laplace_problem.run ();

  return 0;
}
