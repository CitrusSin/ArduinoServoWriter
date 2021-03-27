#include <Servo.h>
#include <LiquidCrystal.h>

//#define RESET_ANGLES
//#define DEBUG

Servo sv1, sv2;
LiquidCrystal lcd(9, 8, 2, 3, 4, 7);

const double arm_len = 1.0;
const double arm_len_sq = arm_len * arm_len;
#ifdef DEBUG
int updatet = 0;
#endif

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
  #ifdef DEBUG
  if (updatet % 100 == 0) {
    lcd.clear();
    lcd.print("Coord:");
    lcd.print(point.x);
    lcd.print(",");
    lcd.print(point.y);
    updatet = 0;
  }
  updatet++;
  #endif
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

void draw_line(Vector2 p1, Vector2 p2) {
  Vector2 line[2] = {p1, p2};
  draw_bezier_curve(line, 2);
}

void draw_polygon(const Vector2 *points, int len) {
  for (int i=1;i<len;i++) {
    draw_line(points[i-1], points[i]);
  }
}

void draw_arc(Vector2 center, double radius, double rad1, double rad2) {
  for (double r=rad1;r<rad2;r+=0.005) {
    apply_coords({center.x+radius*cos(r), center.y+radius*sin(r)});
    delay(2);
  }
}

void setup() {
  Serial.begin(9600);
  sv1.attach(5);
  sv2.attach(6);
  lcd.begin(16, 2);
  lcd.clear();
}

#ifndef RESET_ANGLES

void loop() {
  lcd.clear();
  lcd.print("Hand-Writer v1.0");
  lcd.setCursor(0, 1);
  lcd.print("Circle test");
  while (1) {
    draw_arc({0, 0}, 1, 0, 6.28);
  }
}

#else

void loop() {
  sv1.write(90);
  sv2.write(90);
}

#endif
