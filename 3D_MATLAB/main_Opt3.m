% Main Script
clear;
clc;
%close;


%% Constants (All units in mm)
% Radius of overtube/tool
radius_tool = 1;
% Radius of Scaffold
radius_scaffold = 30;
% Length of the scaffold
length_scaffold = 100;
% Lenght of overtube
length_overtube = 60;
% Distance of back of tool to CG of tool
dist_tool_b_cg = 30;

% Distance of tool tip to CG of Tool
dist_tooltip = 60;


%Tensions
t_min = 5 * ones(6,1);
t_max = 60 * ones(6,1);

% Orientation limits
phi_min = [-10/180 * pi; -10/180 * pi];
phi_max = [10/180 * pi; 10/180 * pi];

% Wrench Vector
W = [0,0,0,0,0,0];
r_ee = [dist_tooltip, 0, 0];
f_ee = [-1,0,0;
         1,0,0;
        0,-1,0;
         0,1,0;
        0,0,-1;
         0,0,1];

%% Taskspace Definition: List of points that form the boundary of the space
% that the surgeon needs the tool to move in for the operation.
taskspace = [];
taskspace(:,end+1) = [20, 5, 5];
taskspace(:,end+1) = [20, -5, 5];
taskspace(:,end+1) = [20, -5, -5];
taskspace(:,end+1) = [20, 5, -5];
taskspace(:,end+1) = [25, 5, -6];
taskspace(:,end+1) = [25, 5, 6];
taskspace(:,end+1) = [25, -5, -6];
taskspace(:,end+1) = [25, -5, 6];
taskspace(:,end+1) = [30, 5, -5];
taskspace(:,end+1) = [35, 5, -10];
taskspace(:,end+1) = [30, 5, 5];
taskspace(:,end+1) = [30, -5, -5];
taskspace(:,end+1) = [30, -5, 5];

% Based on the tooltip, find the poses that the CG of the overtube+tool is
% required to reach.
for i=1:size(taskspace,2)
    taskspace_translated(:,i) = taskspace(:,i) - [dist_tooltip; 0; 0];
end
taskspace_d = taskspace_translated;


%% Design Vector
eaB_min = ones(18,1);
eaB_max = ones(18,1);

%Limits for e
eaB_min(1:6) = eaB_min(1:6) * 0;
eaB_max(1:6) = eaB_max(1:6) * 2 * pi;

%Limits for a
% Front 3 Tendons
eaB_min(7:9) = eaB_min(7:9) * (-dist_tool_b_cg);
eaB_max(7:9) = eaB_max(7:9) * 0;
% Back 3 Tendons
eaB_min(10:12) = eaB_min(10:12) * 0;
eaB_max(10:12) = eaB_max(10:12) * (length_overtube - dist_tool_b_cg);

%Limits for B
% Front 3 Tendons
eaB_min(13:15) = eaB_min(13:15) * (-length_scaffold);
eaB_max(13:15) = eaB_max(13:15) * -(length_scaffold/2);
% Back 3 Tendons
eaB_min(16:18) = eaB_min(16:18) * (-length_scaffold)/2;
eaB_max(16:18) = eaB_max(16:18) * 0;

x0 = [0/180 * pi; 120/180 * pi; 240/180 * pi; 0/180 * pi; 120/180 * pi; 240/180 * pi; ...
    -dist_tool_b_cg; -dist_tool_b_cg; -dist_tool_b_cg; -dist_tool_b_cg + length_overtube;-dist_tool_b_cg + length_overtube;-dist_tool_b_cg + length_overtube; ...
    -length_scaffold; -length_scaffold; -length_scaffold; 0; 0; 0];


%% Pattern Search
%[eaB, fval] = patternsearch(@(x) -my_objective_function(x, W, f_ee, r_ee, phi_min, phi_max, t_min, t_max, taskspace_d/1000, radius_tool, radius_scaffold), x0, [], [], [], [], eaB_min, eaB_max);

%% Particle Swarm Optimization
nvars = 18;
fun = @(x) -my_objective_function2(x, W, f_ee, r_ee, phi_min, phi_max, t_min, t_max, taskspace_d/1000, radius_tool, radius_scaffold);
options = optimoptions('particleswarm','SwarmSize',100,'HybridFcn','patternsearch');
[eaB,fval,exitflag] = particleswarm(fun,nvars,eaB_min,eaB_max,options);

%% Check if Feasible Parameters are found and plotting.
if fval < 0

    % Euler Angles
    ea(1:6) = eaB(1:6);

    % Attachment Points
    a(:,1) = [eaB(7), radius_tool * cos(ea(1)), radius_tool * sin(ea(1))];
    a(:,2) = [eaB(8), radius_tool * cos(ea(2)), radius_tool * sin(ea(2))];
    a(:,3) = [eaB(9), radius_tool * cos(ea(3)), radius_tool * sin(ea(3))];
    a(:,4) = [eaB(10), radius_tool * cos(ea(4)), radius_tool * sin(ea(4))];
    a(:,5) = [eaB(11), radius_tool * cos(ea(5)), radius_tool * sin(ea(5))];
    a(:,6) = [eaB(12), radius_tool * cos(ea(6)), radius_tool * sin(ea(6))];

    % Base Frame 
    B(:,1) = [eaB(13), radius_scaffold * cos(ea(1)), radius_scaffold * sin(ea(1))];
    B(:,2) = [eaB(14), radius_scaffold * cos(ea(2)), radius_scaffold * sin(ea(2))];
    B(:,3) = [eaB(15), radius_scaffold * cos(ea(3)), radius_scaffold * sin(ea(3))];
    B(:,4) = [eaB(16), radius_scaffold * cos(ea(4)), radius_scaffold * sin(ea(4))];
    B(:,5) = [eaB(17), radius_scaffold * cos(ea(5)), radius_scaffold * sin(ea(5))];
    B(:,6) = [eaB(18), radius_scaffold * cos(ea(6)), radius_scaffold * sin(ea(6))];

    draw_cyclops_full(a,B, taskspace, radius_tool, radius_scaffold, length_scaffold, length_overtube, dist_tool_b_cg, dist_tooltip);

    % Workspace Calculation
    f_ee = [0,0,0];
    W = [0,0,0,0,0,0];
    [wp_size, feasible, unfeasible, t] = dex_workspace(a/1000, B/1000, W, f_ee, r_ee/1000, phi_min, phi_max, t_min, t_max);

    % plot
    figure;
    hold;

    plot3(feasible(1,:), feasible(2,:), feasible(3,:), 'bo');
    plot3(unfeasible(1,:), unfeasible(2,:), unfeasible(3,:), 'r.');

    axis([(min(unfeasible(1,:)) - 20/1000) (max(unfeasible(1,:)) + 20/1000) -(radius_scaffold+5)/1000 (radius_scaffold+5)/1000 -(radius_scaffold+5)/1000 (radius_scaffold+5)/1000]);
else
    X = sprintf('Taskspace not feasible given scaffold and overtube size');
    disp(X);
    
end
