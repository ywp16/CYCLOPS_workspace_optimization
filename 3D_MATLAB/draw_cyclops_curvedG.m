function draw_cyclops_curvedG(eaB, taskspace, radius_tool, radius_scaffold)

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
B(:,2) = [eaB(13), radius_scaffold * cos(ea(2)), radius_scaffold * sin(ea(2))];
B(:,3) = [eaB(13), radius_scaffold * cos(ea(3)), radius_scaffold * sin(ea(3))];
B(:,4) = [eaB(14), radius_scaffold * cos(ea(4)), radius_scaffold * sin(ea(4))];
B(:,5) = [eaB(14), radius_scaffold * cos(ea(5)), radius_scaffold * sin(ea(5))];
B(:,6) = [eaB(14), radius_scaffold * cos(ea(6)), radius_scaffold * sin(ea(6))];


add_cables = (size(eaB,1) - 19) / 3;

for i=1:add_cables
    ea(end+1) = eaB(20+(i-1)*3);
    a(:,end+1) = [eaB(21+(i-1)*3), radius_tool * cos(ea(end)), radius_tool * sin(ea(end))];
    B(:,end+1) = [eaB(22+(i-1)*3), radius_scaffold * cos(ea(end)), radius_scaffold * sin(ea(end))];
end

%Find r_ee
gamma_y = eaB(16);
gamma_z = eaB(17);
r_ee_x = eaB(15);

R_y = [cos(gamma_y), 0, sin(gamma_y); 0, 1, 0; -sin(gamma_y), 0, cos(gamma_y)];
R_z = [cos(gamma_z), -sin(gamma_z), 0; sin(gamma_z), cos(gamma_z), 0; 0, 0, 1];
R = R_z * R_y;

r_ee_dir = R * [1;0;0];
r_ee_dir_x = r_ee_dir(1);
r_ee = r_ee_dir / r_ee_dir_x * r_ee_x;

% delta is the vector for the tool tip.
delta_y = eaB(18);
delta_z = eaB(19);

R_y = [cos(delta_y), 0, sin(delta_y); 0, 1, 0; -sin(delta_y), 0, cos(delta_y)];
R_z = [cos(delta_z), -sin(delta_z), 0; sin(delta_z), cos(delta_z), 0; 0, 0, 1];
R = R_z * R_y;

tip_vec = R * [1;0;0];
tip_vec = tip_vec/norm(tip_vec) * 10;

tip_start = r_ee - tip_vec;

curve_vec = [max(a(1,:)) + 5;0;0];



x_middle = (min(B(1,:)) + max(B(1,:)))/2;
p = [x_middle; 0; 0];

length_scaffold = abs(eaB(13) - eaB(14));
length_overtube = max(a(1,:)) - min(a(1,:));

dist_tool_b_cg = abs(min(a(1,:)));


figure;
hold;

% Draw scaffold;
[y,z,x] = cylinder(radius_scaffold, 20);
x = x * -length_scaffold + max(B(1,:));
surf(x,y,z, 'FaceAlpha', 0.1, 'EdgeColor', 'none', 'FaceColor', '[1,0.5,0.5]');

% Draw Overtube;
[y,z,x] = cylinder(radius_tool, 20);
x = x * length_overtube + x_middle + min(a(1,:));
s = surf(x,y,z, 'EdgeColor', 'none', 'FaceColor','[0.5,0.5,0.5]');

% Draw tool to curve point

x = [x_middle,curve_vec(1) + x_middle];
y = [0, 0];
z = [0,0];
plot3(x, y, z, 'k-', 'LineWidth', 1);

% % Draw tool from curve point to tip

x = [curve_vec(1) + x_middle, tip_start(1)+x_middle];
y = [0, tip_start(2)];
z = [0, tip_start(3)];
plot3(x, y, z, 'k-', 'LineWidth', 1);

x = [tip_start(1)+x_middle, r_ee(1)+x_middle];
y = [tip_start(2), r_ee(2)];
z = [tip_start(3), r_ee(3)];
plot3(x, y, z, 'k-', 'LineWidth', 1);



% Draw Tendons
for i=1:size(a,2)
    a_b = a(:,i) + p;
    b = B(:,i);
    
    x = [a_b(1), b(1)];
    y = [a_b(2), b(2)];
    z = [a_b(3), b(3)];
    
    plot3(x, y, z, 'r-');
    plot3(x, y, z, 'rx');
end

% Draw Taskspace
if size(taskspace > 0)
    plot3(taskspace(1,:), taskspace(2,:), taskspace(3,:), 'b.');
end

max_tp_x = max(taskspace(1,:));
max_x_axis = max([max_tp_x, r_ee(1)+x_middle]);

%axis([min(B(1,:))-5 (max_x_axis+10) -(radius_scaffold+5) radius_scaffold+5 -(radius_scaffold+5) radius_scaffold+5 ]);
%axis([-85 50 -(radius_scaffold+5) radius_scaffold+5 -(radius_scaffold+5) radius_scaffold+5 ]);

end