load('data_r_tp.mat');
taskspace = data_r_tp;
taskspace(1,:) = taskspace(1,:) + abs(min(taskspace(1,:))) + 5;
taskspace(2,:) = taskspace(2,:) + radius_scaffold/2;
taskspace(3,:) = taskspace(3,:) + abs(min(taskspace(3,:))) - 22.7951 - 3;
data1 = taskspace;

radius_tool = 1.75;
radius_scaffold = 26.3215;

data_t1 = [];
R = [];

r_curve = [eaB(18); 0; 0];

gamma_y = eaB(16);
gamma_z = eaB(17);

curve_length_x = eaB(15) - eaB(18);

for i=1:size(data1,2)
    alpha_y = data1(5,i);
    alpha_z = data1(6,i);
    
    R_y = [cos(alpha_y), 0, sin(alpha_y); 0, 1, 0; -sin(alpha_y), 0, cos(alpha_y)];
    R_z = [cos(alpha_z), -sin(alpha_z), 0; sin(alpha_z), cos(alpha_z), 0; 0, 0, 1];
    R = R_z * R_y;
    
    temp = R * [1;0;0];
    temp_x = temp(1);
    temp = temp/ temp_x * curve_length_x;
    
    data_t1(1:3,i) = -temp + data1(1:3,i);
    
    beta_y = alpha_y - gamma_y;
    beta_z = alpha_z - gamma_z;
    
    R_y_2 = [cos(beta_y), 0, sin(beta_y); 0, 1, 0; -sin(beta_y), 0, cos(beta_y)];
    R_z_2 = [cos(beta_z), -sin(beta_z), 0; sin(beta_z), cos(beta_z), 0; 0, 0, 1];
    R_2 = R_z_2 * R_y_2;
    
    temp2 = R_2 * r_curve;
    data_t2(1:3,i) = -temp2 + data_t1(1:3,i);
    data_t2(4:6,i) = [0;beta_y;beta_z];
end

%draw_cyclops_full(a,B, taskspace, radius_tool, radius_scaffold, length_scaffold, length_overtube, dist_tool_b_cg, dist_tooltip);
draw_cyclops_curved(eaB, taskspace, radius_tool, radius_scaffold);
%taskspace2 = data_t1;
%plot3(taskspace2(1,:), taskspace2(2,:), taskspace2(3,:), 'r.');
taskspace3 = data_t2;
plot3(taskspace3(1,:), taskspace3(2,:), taskspace3(3,:), 'g.');
