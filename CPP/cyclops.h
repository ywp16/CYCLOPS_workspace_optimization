/*
This contains various functions related to the CYCLOPS.
*/

#ifndef CYCLOPS_H
#define CYCLOPS_H

#include <iostream>
#include <vector>
#include <cmath>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Eigen/Eigen"
#include "linprog.h"


#define PI 3.141592654

namespace cyclops
{
using namespace Eigen;
using namespace std;

Matrix3d test_function(int num_vars, Vector3d position);

// Checks if a given pose is feasible given the attachment and feeding points
bool feasible_pose(Matrix<double, 5,1> P, Matrix<double,3,Dynamic> a, 
	               Matrix<double,3,Dynamic> B, Matrix<double,6,1> W,
	               Vector3d f_ee, Vector3d r_ee,
	               VectorXd t_min, VectorXd t_max);

// Structure to hold results of dextrous workspace function
struct dw_result {
	double size;
	vector<Vector3d> feasible;
	vector<Vector3d> unfeasible;
};

double x_space_length(Matrix<double,3,Dynamic> a, Matrix<double,3,Dynamic> B, Vector3d p);

dw_result dex_workspace(Matrix<double,3,Dynamic> a, Matrix<double,3,Dynamic> B,
	                    Matrix<double,6,1> W, vector<Vector3d> f_ee_vec, 
	                    Vector3d r_ee, Vector2d phi_min, Vector2d phi_max,
	                    VectorXd t_min, VectorXd t_max,
	                    double length_scaffold);

struct fnInputs {
	Matrix<double, 6, 1> W;
	vector<Vector3d> f_ee_vec;
	Vector2d phi_min;
	Vector2d phi_max;
	VectorXd t_min;
	VectorXd t_max;
    vector<VectorXd> taskspace;
    double radius_tool;
    double radius_scaffold;
    double length_scaffold;
    int curve_tool;
};

// For Dex taskspace - Straight tool
double objective_function(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);

// For Taskspace with orientation (wrt tooltip) - straight tool
double objective_function2(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);

// For Taskspace with orientation (wrt tooltip) - curved tool
double objective_function2c(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);

double objective_function2cG(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);

// For Taskspace with orientation (wrt tooltip) - curved tool -> double curve
double objective_function2c2(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);


// For Taskspace with orientation and forces (wrt tooltip) - curved tool
double objective_function3c(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);

double objective_function3cG(Matrix<double,Dynamic,1> eaB, Matrix<double,6,1> W,
	                      vector<Vector3d> f_ee_vec,
	                      Vector2d phi_min, Vector2d phi_max,
	                      VectorXd t_min, VectorXd t_max,
	                      vector<VectorXd> taskspace,
	                      double radius_tool, double radius_scaffold,
	                      double length_scaffold);

}; //namespace cyclops

#endif // CYCLOPS_H