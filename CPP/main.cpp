/*
Imperial College London Msc Individual Project by Yi Wei Pang
Main file


Acknowledgements:

Modified and used code from the following sources:

SIMPSOLIB: http://profesores.elo.utfsm.cl/~tarredondo/simgalib.html
Eigen: http://eigen.tuxfamily.org/index.php?title=Main_Page
libigl: https://github.com/libigl/libigl

Implemented algorithms based on information from the following:

Optimization of a planar tendon-driven parallel manipulator for a maximal dextrous workspace (Hay and Snyman, 2005)
Particle Filter with Swarm Move for Optimization (Zhang and Tong et al, 2008)
A particle swarm pattern search method for bound constrained global optimization (Vaz, A. Ismael F. and Vicente, Luís N., 2007)


*/
#include <iostream>
#include <ctime>
#include <cmath>
#include <queue>
#include "simpsolib.h"
#include "simtstlib.h"
#include <fstream>
#include <string>

#include "Eigen/Eigen"
#include "cyclops.h"

#define PI 3.141592654
#define g 9.81

using namespace simpsolib;
using namespace simtstlib;

int main(int argc, char *argv[])
{
    string input_filename = "./input.txt";
    string tp_filename = "./taskspace.txt";

    if (argc > 2)
    {
        std::cout << "Input File: " << argv[1] << std::endl;
        std::cout << "Taskspace File: " << argv[2] << std::endl << std::endl;

        input_filename = argv[1];
        tp_filename = argv[2];
    }
    
    string temp;
    ifstream inData;
    inData.open(input_filename.c_str());
    inData >> temp; //Take the in the heading word

    //---- Defining CYCLOPS parameters ----
    cyclops::fnInputs fnInputs;
    // Radius of Overtube
    double radius_tool = 2.0;
    inData >> temp >> radius_tool;
    fnInputs.radius_tool = radius_tool;
    // Radius of Scaffold
    double radius_scaffold = 30.0;
    inData >> temp >> radius_scaffold;
    fnInputs.radius_scaffold = radius_scaffold;
    // Lenght of Scaffold
    double length_scaffold = 100.0;
    inData >> temp >> length_scaffold;
    fnInputs.length_scaffold = length_scaffold;
    // Length of Overtube
    double length_overtube = 60.0;
    inData >> temp >> length_overtube;
    // Distance of back of tool to CG of tool
    double dist_tool_b_cg = 30.0;
    inData >> temp >> dist_tool_b_cg;
    // Number of Tendons
    int num_tendons = 8;
    inData >> temp >> num_tendons;

    // Is the tool curved?
    int curve = 0;
    inData >> temp >> curve;
    if (curve > 2)
    {
        fnInputs.curve_tool = 2;
        curve = 2;
    }
    else
    {
        fnInputs.curve_tool = curve;
    }
    
    std::cout << "CYCLOPS PARAMS" << std::endl;
    std::cout << "Radius Tool: " << radius_tool << std::endl;
    std::cout << "Radius Scaffold: " << radius_scaffold << std::endl;
    std::cout << "Length Scaffold: " << length_scaffold << std::endl;
    std::cout << "Length Overtube: " << length_overtube << std::endl;
    std::cout << "Distance Tool Back: " << dist_tool_b_cg << std::endl;
    std::cout << "Number of Tendons: " << num_tendons << std::endl;
    std::cout << "Curved Tool: " << fnInputs.curve_tool << std::endl;
    
    // Tensions
    double tension_min = 5.0;
    double tension_max = 60.0;
    inData >> temp >> tension_min >> temp >> tension_max;

    std::cout << "Min. Tension: " << tension_min << " Max. Tension: " << tension_max << std::endl;

    // Orientation Limits
    double orientation_limit = 10.0; // in degrees
    inData >> temp >> orientation_limit;

    std::cout << "Orientation Limit: " << orientation_limit << std::endl;

    // External Wrench
    Eigen::Matrix<double,6,1> W;
    inData >> temp;
    for (int i=0; i<6; i++)
    {
        double temp_wrench_scalar;
        inData >> temp_wrench_scalar;
        W(i,0) = temp_wrench_scalar;
    }
    //W << 0.0,0.0,0.0,0.0,0.0,0.0;
    std::cout << "Wrench Vector: " << W.transpose() << std::endl;
    // Mass of tool and CG Position
    double mass = 0.0;
    double CG_pos = 0.0;
    inData >> temp >> mass >> temp >> CG_pos;

    std::cout << "Mass of Tool: " << mass << std::endl;
    std::cout << "CG of Tool: " << CG_pos << std::endl;

    // End Effector Force 
    int f_ee_vec_size;
    
    inData >> temp >> f_ee_vec_size;
    for (int i=0; i<f_ee_vec_size; i++)
    {
        Eigen::Vector3d f_ee_temp;
        double f_ee_scalar;
        inData >> f_ee_scalar;
        f_ee_temp(0) = f_ee_scalar;
        inData >> f_ee_scalar;
        f_ee_temp(1) = f_ee_scalar;
        inData >> f_ee_scalar;
        f_ee_temp(2) = f_ee_scalar;

        fnInputs.f_ee_vec.push_back(f_ee_temp);        
    }
    std::cout << "End Effector Force Vector Size: " << fnInputs.f_ee_vec.size() << std::endl;

    // Tool Tip Max Length(from cg)
    double tool_tip_limit = 70.0;
    inData >> temp >> tool_tip_limit;
    std::cout << "Tool Tip Length Limit: " << tool_tip_limit << std::endl;

    // Angle limits for tendons to be attached in scaffold
    double angle_lower_limit = 0.0;
    double angle_upper_limit = 360.0;
    inData >> temp >> angle_lower_limit >> temp >> angle_upper_limit;
    std::cout << "Angle Range: " << angle_lower_limit << " - " << angle_upper_limit << std::endl << std::endl;



//---------------------------------------------------------------------------------
// Creating Matrices and Vectors for CYCLOPS optimization (TRY NOT TO TOUCH)
//---------------------------------------------------------------------------------
    Eigen::VectorXd t_min;
    t_min.resize(num_tendons, 1);
    Eigen::VectorXd t_max;
    t_max.resize(num_tendons, 1);

    for (int i=0;i<num_tendons;i++)
    {
        t_min(i,0) = tension_min;
        t_max(i,0) = tension_max;
    }
    fnInputs.t_min = t_min;
    fnInputs.t_max = t_max;


    // Orientation Limits
    Eigen::Vector2d phi_min;
    phi_min << -orientation_limit/180.0 * PI, -orientation_limit/180.0 * PI;
    fnInputs.phi_min = phi_min;
    Eigen::Vector2d phi_max;
    phi_max << orientation_limit/180.0 * PI, orientation_limit/180.0 * PI;
    fnInputs.phi_max = phi_max;


    // Wrench Vector
    Eigen::Vector3d r_cg, f_cg;
    f_cg << 0.0, mass * -g, 0.0;
    r_cg << CG_pos, 0.0, 0.0;
    Eigen::Vector3d tau_f_cg = f_cg.cross(r_cg/1000);
    Eigen::Matrix<double,6,1> W_cg;
    W_cg << f_cg(0,0), f_cg(1,0), f_cg(2,0), tau_f_cg(0,0), tau_f_cg(1,0), tau_f_cg(2,0);
    fnInputs.W = W + W_cg;

   // cout << (fnInputs.W).transpose() << endl;


    // Taskspace Definition
    
    ifstream inTaskspace;
    inTaskspace.open(tp_filename.c_str());
    inTaskspace >> temp; //Take the in the heading word

    Eigen::VectorXd tp_temp;

    int TP_type;
    inTaskspace >> TP_type;

    if (TP_type == 1)
    {
        tp_temp.resize(3,1);

        while (!inTaskspace.eof())
        {
            double tp_temp_scalar;
            inTaskspace >> tp_temp_scalar;
            if( inTaskspace.eof() ) break;

            tp_temp(0) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(1) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(2) = tp_temp_scalar;

            fnInputs.taskspace.push_back(tp_temp/1000.0);
            //std::cout << tp_temp.transpose() << std::endl;
        }
    }
    else if (TP_type == 2)
    {
        tp_temp.resize(5,1);

        while (!inTaskspace.eof())
        {
            double tp_temp_scalar;
            inTaskspace >> tp_temp_scalar;
            if( inTaskspace.eof() ) break;

            tp_temp(0) = tp_temp_scalar/1000.0;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(1) = tp_temp_scalar/1000.0;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(2) = tp_temp_scalar/1000.0;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(3) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(4) = tp_temp_scalar;

            fnInputs.taskspace.push_back(tp_temp);
            //std::cout << tp_temp.transpose() << std::endl;
        }
    }
    else if (TP_type == 3)
    {
        tp_temp.resize(8,1);

        while (!inTaskspace.eof())
        {
            double tp_temp_scalar;
            inTaskspace >> tp_temp_scalar;
            if( inTaskspace.eof() ) break;

            tp_temp(0) = tp_temp_scalar/1000.0;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(1) = tp_temp_scalar/1000.0;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(2) = tp_temp_scalar/1000.0;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(3) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(4) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(5) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(6) = tp_temp_scalar;
            inTaskspace >>  tp_temp_scalar;
            tp_temp(7) = tp_temp_scalar;

            fnInputs.taskspace.push_back(tp_temp);
            //std::cout << tp_temp.transpose() << std::endl;
        }
    }

    std::cout << "No. of points in taskspace: " << fnInputs.taskspace.size() << std::endl <<std::endl;
    

    // Design Vector and limits
    vector<double> lower_range;
    vector<double> upper_range;

    lower_range.resize((num_tendons - 6)*3 + 15 + curve *3);
    upper_range.resize((num_tendons - 6)*3 + 15 + curve *3);

    // Limits for basic six tendons
    // Limits for e (angles)
    for (int i=0; i<6; i++)
    {
        lower_range[i]= angle_lower_limit/180.0 * PI;
        upper_range[i]= angle_upper_limit/180.0 * PI;
    }
    // Limits for a
    /*
    for (int i=6; i<12; i++)
    {
        lower_range[i] = -dist_tool_b_cg;
        upper_range[i] = (length_overtube - dist_tool_b_cg);
    }*/
    

    // Using halfspace method
    // Back 3 tendons
    for (int i=6; i<9; i++)
    {
        lower_range[i] = -dist_tool_b_cg;
        upper_range[i] = 0.0;
    }
    // Front 3 Tendons
    for (int i=9; i<12; i++)
    {
        lower_range[i] = 0.0;
        upper_range[i] = (length_overtube - dist_tool_b_cg);
    }
    
    // Limits for B
    
    for (int i=12; i<14; i++)
    {
        lower_range[i] = -length_scaffold;
        upper_range[i] = 0.0;
    }
/*
    //Using halfsapce method
   
    lower_range[12] = -length_scaffold;
    upper_range[12] = -length_scaffold/2.0;
    lower_range[13] = -length_scaffold/2.0;
    upper_range[13] = 0.0;
    */

    // Limits for tooltip r_ee_x
    lower_range[14] = 0.0;
    upper_range[14] = tool_tip_limit;

   /*
    if (curve == 1)
    {
        for (int i=0; i<curve;i++)
        {
            lower_range[15+i*3] = -PI/2;
            upper_range[15+i*3] = PI/2;
            lower_range[16+i*3] = -PI/2;
            upper_range[16+i*3] = PI/2;
            lower_range[17+i*3] = 0.0;
            upper_range[17+i*3] = tool_tip_limit;
        }
    }
*/
    if (curve == 1)
    {
        lower_range[15] = -PI/2;
        upper_range[15] = PI/2;
        lower_range[16] = -PI/2;
        upper_range[16] = PI/2;
        lower_range[17] = 0.0;
        upper_range[17] = tool_tip_limit;
    }
    else if (curve == 2)
    {
        lower_range[15] = -PI/4;
        upper_range[15] = PI/4;
        lower_range[16] = -PI/4;
        upper_range[16] = PI/4;
        lower_range[17] = -PI/4;
        upper_range[17] = PI/4;
        lower_range[18] = -PI/4;
        upper_range[18] = PI/4;
    }

    int temp_c = 0;
    if (curve == 0)
    {
        temp_c = 0;
    }
    else
    {
        temp_c = 2;
    }

    // Limits for additional Tendons
    for (int i=0; i<num_tendons-6; i++)
    {
        lower_range[15+i*3+curve+temp_c] = angle_lower_limit/180.0 * PI;
        upper_range[15+i*3+curve+temp_c] = angle_upper_limit/180.0 * PI;
        lower_range[15+i*3+1+curve+temp_c] = -dist_tool_b_cg;
        upper_range[15+i*3+1+curve+temp_c] = (length_overtube - dist_tool_b_cg);
        lower_range[15+i*3+2+curve+temp_c] = -length_scaffold;
        upper_range[15+i*3+2+curve+temp_c] = 0.0;
    }

/*
    // Limits for additional Tendons
    for (int i=0; i<num_tendons-6; i++)
    {
        lower_range[15+i*3+curve*3] = angle_lower_limit/180.0 * PI;
        upper_range[15+i*3+curve*3] = angle_upper_limit/180.0 * PI;
        lower_range[15+i*3+1+curve*3] = -dist_tool_b_cg;
        upper_range[15+i*3+1+curve*3] = (length_overtube - dist_tool_b_cg);
        lower_range[15+i*3+2+curve*3] = -length_scaffold;
        upper_range[15+i*3+2+curve*3] = 0.0;
    }
*/
//---------------------------------------------------------------------------------
// Optimization: PSO + SA + PFO
//---------------------------------------------------------------------------------

    inData >> temp;
    //std::cout << temp << std::endl;
    // Parameters for PSO
    int number_runs=5;
    int pso_pop=500;
    int pso_iters=50;

    inData >> temp >> number_runs >> temp >> pso_pop >> temp >> pso_iters;

    float phi_p=0.50;
    float phi_g=0.50;;
    double omega_initial=0.90;
    double omega_final=0.50;
    bool rand_update=false; 

    // Parameters for PFO
    int pfo_pop=50;
    int pfo_iters=30;
    int pfo_resample_factor = 10;

    inData >> temp >> pfo_pop >> temp >> pfo_iters >> temp >> pfo_resample_factor; 

    std::cout << "OPTIMIZATION PARAMS" << std::endl;
    std::cout << "Number of Runs: " << number_runs << std::endl;
    std::cout << "PSwarm Population Size: " << pso_pop << std::endl;
    std::cout << "PSwarm Number of Iterations: " << pso_iters << std::endl;
    std::cout << "PFO Population Size: " << pfo_pop << std::endl;
    std::cout << "PFO Number of Iterations: " << pfo_iters << std::endl;
    std::cout << "PFO Resample Factor: " << pfo_resample_factor << std::endl << std::endl;

    std::cout << "STARTING OPTIMIZATION" << std::endl;
    int num_dims = 15 + (num_tendons - 6) * 3 + curve + temp_c;
    //Create logging file
    //ofstream cout("output.txt")


    // perform PSO experiment
    //simpsolib::EvalFN pso_eval_fn((char *)"DEJONG2", DEJONG2_FN_NUM_VARS,lower_range, upper_range, dejong2_test_fn_real);
    simpsolib::EvalFN pso_eval_fn((char *)"CYCLOPS_OPT", num_dims, lower_range, upper_range, dejong2_test_fn_real);
    pso_eval_fn.Input = fnInputs;
    run_pso(pso_eval_fn, number_runs, pso_pop, pso_iters, phi_p, phi_g, omega_initial, omega_final, rand_update, pfo_pop, pfo_iters, pfo_resample_factor);
    

/*

    Eigen::Matrix<double,3,6> a, B;
    
    a << 30.0, 30.0, 30.0, -30.0, -30.0, -30.0, 0.0,
         0.5, -1.0, 0.5, 0.5, -1.0, 0.5, 1.0,
         0.866, 0.0, -0.866, 0.866, 0.0, -0.866, 0.0;
    B << 0.0, 0.0, 0.0, -100.0, -100.0, -100.0, -50.0,
         15.0, -30.0, 15.0, 15.0, -30.0, 15.0, 30.0,
         25.98, 0.0, -25.98, 25.98, 0.0, -25.98, 0.0; 

//    a << -7.2716,  -11.3936,  -21.7060,   11.5591,  16.7089,   19.7650,
//   -0.6269,    0.9821,   -0.2583,   -0.3502,    1.0000,   -0.4169,
//   -0.7791,   -0.1882,    0.9661,   -0.9367,   -0.0000,    0.9090;

//    B << -85.1078, -85.1078, -85.1078, -4.9541, -4.9541, -4.9541,
//    -18.8056, 29.4638, -7.7491, -10.5072, 30.0000, -12.5071,
//    -23.3741, -5.6468, 28.9819, -28.0998, -0.0000, 27.2685;

 //   a << -9.79608,-14.0274,-15.1943,18.4967,21.3164,15.5718,0,
 //   -0.898808,-0.108916,0.992476,-0.330122,-0.782099,-0.304015,1,
 //   -0.438343,0.994051,-0.122438,-0.943938,0.623154,-0.952667,0;
    
 //   B << 0,0,0,0,0,0,0,
 //   -26.9642,-3.26749,29.7743,-9.90367,-23.463,-9.12044,30,
 //   -13.1503,29.8215,-3.67315,-28.3181,18.6946,-28.58,0;
    


    Eigen::Vector3d f_ee;
    Eigen::Vector3d r_ee;
    f_ee << 0.0, 0.0, 0.0;
    r_ee << 70.0, 0.0, 0.0;
    Eigen::Matrix<double,6,1> W;
    W << 0.0,0.0,0.0,0.0,0.0,0.0;
    std::vector<Eigen::Vector3d> f_ee_vec;
    f_ee_vec.push_back(f_ee);
    
    std::cout << "f_ee is: " << f_ee << std::endl << std::endl;


    Eigen::Vector3d f_ee;
    Eigen::Vector3d r_ee;
    f_ee << 0.0, 0.0, 0.0;
    r_ee << 100.0, 0.0, 0.0;


    Eigen::Matrix<double,5,1> P;
    //P << -50.0, 0.0, 0.0, 0.0, 0.0;
    //P << -200, 0, 0, 0, 0;
    //P << -0.0660, -0.0060, -0.0000, 0, 0;
    P << -94.909,  -9.543,   3.704,   0.019,   0.325;

    Eigen::Matrix<double, 6, 1> t_min, t_max;
    t_min << 5.0,5.0,5.0,5.0,5.0,5.0;
    t_max << 60.0,60.0,60.0,60.0,60.0,60.0;
    
    Eigen::Vector2d phi_min, phi_max;
    phi_min << -10.0/180.0 * PI, -10.0/180.0 * PI;
    phi_max << 10.0/180.0 * PI, 10.0/180.0 * PI; 

    bool result = cyclops::feasible_pose(P/1000.0, a/1000.0, B/1000.0, W, f_ee, r_ee/1000.0, t_min, t_max);

    std::cout << "Feasibility of Given Pose: " << result << std::endl;

    cyclops::dw_result test_dw = cyclops::dex_workspace(a/1000.0, B/1000.0, W, f_ee_vec, r_ee/1000.0, phi_min, phi_max, t_min, t_max, length_scaffold/1000);
    

    std::cout << "No. of Feasible pts is: " << (test_dw.feasible).size() << std::endl;
    std::cout << "No. of UnFeasible pts is: " << (test_dw.unfeasible).size() << std::endl;
    std::cout << "Workspace Size: " << (test_dw.size) << std::endl;

    std::vector<Eigen::Vector3d> feasible = test_dw.feasible;
    std::vector<Eigen::Vector3d> unfeasible = test_dw.unfeasible;

    std::vector<Eigen::Vector3d>::iterator feasible_iter;
    std::vector<Eigen::Vector3d>::iterator unfeasible_iter;

    std::cout << "Positions:" << std::endl;
    for(feasible_iter=feasible.begin(); feasible_iter!=feasible.end(); ++feasible_iter)
    {
        std::cout << (*feasible_iter)(0) << ", " << (*feasible_iter)(1) << ", " << (*feasible_iter)(2) << ";" << std::endl;
    }




    Eigen::Matrix<double, 18, 1> eaB;
    
    eaB << -3.2535, -1.1420, 0.5236, -3.6652, 0.5236, -1.2702,
           -10.4870, -7.6943, -9.7298, 14.9779, 5.4373, 5.5362,
           -119.9850, 0,
           100.7753, 0.3, 0.2,
            25.0000;

    //eaB << 0.5236, -1.0257, -2.1287, -1.4468, 0.5150, -3.6652,
    //       -13.0146, -2.9159, -15.1972, 6.3325, 3.6564, 26.4041,
    //       -91.4513, -12.2077,
    //       89.3397, -0.7930, -0.5890, 35.5969;

    double val = cyclops::objective_function2c(eaB, fnInputs.W, fnInputs.f_ee_vec, fnInputs.phi_min, fnInputs.phi_max,
                          fnInputs.t_min, fnInputs.t_max, fnInputs.taskspace, fnInputs.radius_tool, 
                          fnInputs.radius_scaffold, fnInputs.length_scaffold);

    std::cout << "Value of Objective Function is: " << val << std::endl;



    Eigen::Matrix<double, 21,1> eaB;

    eaB << 1.0472, 3.1416, 5.2360, 1.0472, 3.1416, 5.2360,
           2.9000, 2.9000, 2.9000, -2.9000, -2.9000, -2.9000,
           0, -72.2100, 56.7100,
           0.2820, 0.2018, 7.9000,
           -0.2820, -0.2018, 50.0000;

    double val = cyclops::objective_function2c2(eaB, fnInputs.W, fnInputs.f_ee_vec, fnInputs.phi_min, fnInputs.phi_max,
                          fnInputs.t_min, fnInputs.t_max, fnInputs.taskspace, fnInputs.radius_tool, 
                          fnInputs.radius_scaffold, fnInputs.length_scaffold);

    std::cout << "Value of Objective Function is: " << val << std::endl;

*/
    return 0;
}
