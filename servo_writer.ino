#include <Servo.h>

//#define RESET_ANGLES

Servo sv1, sv2;
const double arm_len = 1.0;

const double arm_len_sq = arm_len * arm_len;

typedef struct Vector2 {
  double x;
  double y;
} Vector2;

Vector2 get_servo_angles(Vector2 p) {
  Vector2 point = {p.x*.3, p.y*.3+1.5};
  double r_sq = point.x*point.x+point.y*point.y;
  double cos_d_angle = r_sq/(2*arm_len_sq)-1;
  double d_angle = acos(cos_d_angle);
  double bas_angle = atan2(point.y, point.x);
  Vector2 angles;
  angles.x = bas_angle + d_angle/2;
  angles.y = bas_angle - d_angle/2;
  return angles;
}

void apply_angles(Vector2 angles) {
  sv1.write(angles.x * 57.2975); // 1rad = 57.2975deg
  sv2.write(angles.y * 57.2975);
}

void apply_coords(Vector2 point) {
  apply_angles(get_servo_angles(point));
}

void draw_line(Vector2 p1, Vector2 p2) {
  if (abs(p1.x-p2.x)>0.01) {
    double len = sqrt((p2.x-p1.x)*(p2.x-p1.x)+(p2.y-p1.y)*(p2.y-p1.y));
    double k = (p2.y-p1.y)/(p2.x-p1.x);
    double ct = 1/sqrt(k*k+1), st=ct*k;
    if (p1.x > p2.x) {
      st = -st;
      ct = -ct;
    }
    for (double t=0;t<len;t+=0.01) {
      apply_coords({ct*t+p1.x, st*t+p1.y});
      delay(2);
    }
  } else {
    double dy = (p1.y > p2.y) ? -1.0 : 1.0;
    double len = abs(p2.y-p1.y);
    for (double t=0;t<len;t+=0.01) {
      apply_coords({p1.x, t*dy+p1.y});
      delay(2);
    }
  }
}

void draw_polygon(const Vector2 *points, int len) {
  for (int i=1;i<len;i++) {
    draw_line(points[i-1], points[i]);
  }
}

Vector2 bezier_curve(const Vector2 *control_points, int len, double t) {
  if (len == 2) {
    Vector2 *p1=control_points, *p2=control_points+1;
    double dx = p2->x-p1->x, dy = p2->y-p1->y;
    return {p1->x+dx*t, p1->y+dy*t};
  } else if (len > 2) {
    Vector2 *np = new Vector2[len-1];
    for (int i=0;i<len-1;i++) {
      double dx = control_points[i+1].x-control_points[i].x,
             dy = control_points[i+1].y-control_points[i].y;
      np[i] = {control_points[i].x+dx*t, control_points[i].y+dy*t};
    }
    Vector2 result = bezier_curve(np, len-1, t);
    delete np;
    return result;
  } else {
    return {0, 0};
  }
}

void draw_bezier_curve(const Vector2 *control_points, int len, double p_density = .001) {
  for (double t=0.0;t<=1.0;t+=p_density) {
    apply_coords(bezier_curve(control_points, len, t));
    delayMicroseconds(250);
  }
}

void setup() {
  Serial.begin(9600);
  sv1.attach(5);
  sv2.attach(6);
}

#ifdef RESET_ANGLES

void loop() {
  sv1.write(90);
  sv2.write(90);
}

#else

Vector2 control_points[] =
{
  {-1, 0},
  {1, 0}
};
void loop() {
  draw_bezier_curve(control_points, sizeof(control_points)/sizeof(Vector2));
  delay(1000);
  apply_coords({-1, 0});
  delay(1000);
}

#endif
