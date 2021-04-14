#include <Servo.h>
#include <LiquidCrystal.h>

//#define RESET_ANGLES
#define DEBUG

const char *characters[256];

Servo sv1, sv2, sv_pen;
LiquidCrystal lcd(9, 8, 2, 3, 4, 7); 

const double arm_len = 1.0;
const double arm_len_sq = arm_len * arm_len;
#ifdef DEBUG
int updatet = 0;
#endif

typedef struct vector2 {
  double x;
  double y;
}vector2;

void setup_characters() {
  characters['1'] = "2,34+1,-27";
  characters['2'] = "-26,6+-2,46+18,27+32,0+-18,-41;-18,-42+16,-41+24,-43";
  characters['3'] = "-10,21+0,30+13,23+12,9+0,0;0,0+10,0+20,-20+0,-28+-11,-19";
  characters['4'] = "-4,25+-21,0;-21,0+12,0H;0,13+0,-25";
  characters['5'] = "-10,20+-14,2;-13,2+0,14+12,1+12,-14+0,-21+-14,-15H;-10,20+13,20";
  characters['a'] = "15,9+-1,28+-25,13+-31,-32+-1,-42+14,9;14,9+11,-20+14,-23+18,-19";
  characters['b'] = "-15,34+-17,-20;-17,-20+-17,0+0,15+16,1+16,-26+-1,-29+-23,-14";
  characters['c'] = "7,16+0,27+-39,6+-32,-25+-5,-35+7,-18+15,-10";
}

void attach_servos() {
  sv1.attach(5);
  sv2.attach(6);
  sv_pen.attach(10);
  sv_pen.write(28);
}

void soft_approach_servo(Servo *sv, double fromAngle, double toAngle) {
  for (double nowAngle = fromAngle; abs(nowAngle-toAngle) > 0.05; nowAngle += (toAngle-nowAngle)*0.5) {
    sv->write((int)nowAngle);
    delay(100);
  }
  sv->write(toAngle);
}

#define DROP 0
#define HANG 1
int pen_state = HANG;
void pen(int state) {
  switch (state) {
    case DROP:
      if (pen_state == HANG) {
        soft_approach_servo(&sv_pen, 28, 14);
        pen_state = DROP;
      }
    break;
    case HANG:
      sv_pen.write(28);
      pen_state = HANG;
    break;
  }
  delay(100);
}

vector2 get_servo_angles(vector2 p) {
  vector2 point = {p.x*.3, p.y*.3+1.5};
  double r_sq = point.x*point.x+point.y*point.y;
  double cos_d_angle = r_sq/(2*arm_len_sq)-1;
  double d_angle = acos(cos_d_angle);
  double bas_angle = atan2(point.y, point.x);
  vector2 angles;
  angles.x = bas_angle + d_angle/2;
  angles.y = bas_angle - d_angle/2;
  return angles;
}

void apply_angles(vector2 angles) {
  sv1.write(angles.x * 57.2975); // 1rad = 57.2975deg
  sv2.write(angles.y * 57.2975);
}

void apply_coords(vector2 point) {
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

vector2 bezier_curve(const vector2 *control_points, int len, double t) {
  if (len == 2) {
    vector2 *p1=control_points, *p2=control_points+1;
    double dx = p2->x-p1->x, dy = p2->y-p1->y;
    return {p1->x+dx*t, p1->y+dy*t};
  } else if (len > 2) {
    vector2 *np = new vector2[len-1];
    for (int i=0;i<len-1;i++) {
      double dx = control_points[i+1].x-control_points[i].x,
             dy = control_points[i+1].y-control_points[i].y;
      np[i] = {control_points[i].x+dx*t, control_points[i].y+dy*t};
    }
    vector2 result = bezier_curve(np, len-1, t);
    delete np;
    return result;
  } else {
    return {0, 0};
  }
}

void draw_bezier_curve(const vector2 *control_points, int len, double p_density = .001) {
  if (len > 0) {
    for (double t=0.0;t<=1.0;t+=p_density) {
      apply_coords(bezier_curve(control_points, len, t));
      delayMicroseconds(0);
    }
  }
}

void draw_line(vector2 p1, vector2 p2) {
  vector2 line[2] = {p1, p2};
  draw_bezier_curve(line, 2);
}

void draw_polygon(const vector2 *points, int len) {
  for (int i=1;i<len;i++) {
    draw_line(points[i-1], points[i]);
  }
}

void draw_arc(vector2 center, double radius, double rad1, double rad2) {
  apply_coords({center.x+radius*cos(rad1), center.y+radius*sin(rad1)});
  delay(10);
  for (double r=rad1;r<rad2;r+=0.005) {
    apply_coords({center.x+radius*cos(r), center.y+radius*sin(r)});
    delay(2);
  }
  pen(HANG);
}

void draw_figurestr(const char* figure, vector2 offset={0, 0}) {
  int curve_count = 1;
  for (char *ptr = figure;*ptr!='\0';ptr++) {
    if (*ptr == ';') {
      curve_count++;
    }
  }
  char *str = new char[strlen(figure)+1];
  strcpy(str, figure);
  char **figures = new char*[curve_count];
  int counter = 0;
  char *figures_token = strtok(str, ";");
  while (figures_token) {
    figures[counter] = figures_token;
    counter++;
    figures_token = strtok(NULL, ";");
  }
  for (int i=0;i<curve_count;i++) {
    char *figure = figures[i];
    int vector_count = 1;
    for (char *ptr=figure;*ptr!='\0';ptr++) {
      if (*ptr == '+') {
        vector_count++;
      }
    }
    vector2 *vectors = new vector2[vector_count];
    counter = 0;
    bool hang = false;
    if (figure[strlen(figure)-1] == 'H') {
      hang = true;
      figure[strlen(figure)-1] = '\0';
    }
    char *t = strtok(figure, "+");
    while (t) {
      int i1, i2;
      sscanf(t, "%d,%d", &i1, &i2);
      vectors[counter] = {i1*0.02+offset.x, i2*0.02+offset.y};
      counter++;
      t = strtok(NULL, "+");
    }
    apply_coords(vectors[0]);
    delay(500);
    pen(DROP);
    draw_bezier_curve(vectors, vector_count);
    if (hang) {
      pen(HANG);
    }
    delete vectors;
  }
  delete figures;
  delete str;
  pen(HANG);
}

void setup() {
  setup_characters();
  attach_servos();
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  pen(HANG);
  delay(500);
  apply_coords({0, 0});
}

#ifndef RESET_ANGLES

#ifndef DEBUG
void loop() {
  lcd.clear();
  lcd.print("Hand-Writer v1.0");
  lcd.setCursor(0, 1);
  lcd.print("test");
  delay(5000);
}
#else
void loop() {
  lcd.clear();
  lcd.print("Hand-Writer v1.0");
  lcd.setCursor(0, 1);
  lcd.print("test");
  delay(5000);
  Serial.println("Reading a char");
  while (true) {
    int c = Serial.read();
    if (c > 0 && c != '\n') {
      Serial.print("Writing ");
      Serial.println((char)c);
      draw_figurestr(characters[c]);
      Serial.println("Reading a char");
    }
  }
}
#endif

#else

void loop() {
  sv1.write(90);
  sv2.write(90);
  sv_pen.write(20);
}

#endif
