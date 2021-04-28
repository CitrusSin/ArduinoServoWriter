#include <Servo.h>
#include <LiquidCrystal.h>

//#define RESET_ANGLES
//#define DEBUG

const char *characters[256];

Servo sv1, sv2, sv_pen;
LiquidCrystal lcd(9, 8, 2, 3, 4, 7); 

const double arm_len = 1.0;
const double arm_len_sq = arm_len * arm_len;

typedef struct vector2 {
  double x;
  double y;
}vector2;

void setup_characters() {
  characters['0'] = "0,28+-12,27+-15,0;-15,0+-13,-28+0,-27;0,-26+15,-26+16,-1;16,-1+14,26+0,27";
  characters['1'] = "2,34+1,-27";
  characters['2'] = "-12,7+0,21+9,14+18,0+-8,-21;-8,-21+13,-20";
  characters['3'] = "-12,18+12,21+0,0+-7,0;-6,-1+12,-3+11,-24+-14,-24";
  characters['4'] = "0,25+-21,0;-21,0+23,0H;0,24+0,-22";
  characters['5'] = "-13,24+-14,0;-14,0+0,16+16,-1+11,-23+1,-25+-11,-19H;-13,23+9,24";
  characters['6'] = "12,26+0,38+-20,22+-17,-23+0,-22;0,-22+14,-22+12,0+0,5+-4,5;-4,5+-8,5+-12,-1";
  characters['7'] = "-12,16+12,16;12,16+0,9+0,-14+0,-21";
  characters['8'] = "10,19+-2,39+-19,19+0,0;0,0+20,-20+0,-44+-14,-22;-14,-22+-16,-18+0,0+13,20";
  characters['9'] = "10,10+0,27+-16,13;-16,13+-28,-6+0,-8;0,-8+9,-7+10,9;10,9+9,-45";
  //characters['a'] = "15,9+-1,28+-25,13+-31,-32+-1,-42+14,9;14,9+11,-20+14,-23+18,-19";
  //characters['b'] = "-15,34+-17,-20;-17,-20+-17,0+0,15+16,1+16,-26+-1,-29+-23,-14";
  //characters['c'] = "7,16+0,27+-39,6+-32,-25+-5,-35+7,-18+15,-10";
}

void attach_servos() {
  sv1.attach(5);
  sv2.attach(6);
  sv_pen.attach(10);
  sv_pen.write(31);
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
        soft_approach_servo(&sv_pen, 31, 12);
        pen_state = DROP;
      }
    break;
    case HANG:
      if (pen_state == DROP) {
        sv_pen.write(33);
        delay(100);
        sv_pen.write(31);
        pen_state = HANG;
      }
    break;
  }
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
}

vector2 bezier_curve(const vector2 *control_points, int len, double t) {
  if (len == 2) {
    const vector2 *p1=control_points, *p2=control_points+1;
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

void draw_bezier_curve(const vector2 *control_points, int len, double p_density = .002) {
  if (len > 0) {
    for (double t=0.0;t<=1.0;t+=p_density) {
      apply_coords(bezier_curve(control_points, len, t));
      //delayMicroseconds(10);
    }
    apply_coords(control_points[len-1]);
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
  Serial.println("Using figure string");
  Serial.println(figure);
  int curve_count = 1;
  for (const char *ptr = figure;*ptr!='\0';ptr++) {
    if (*ptr == ';') {
      curve_count++;
    }
  }
  Serial.print("Curve count ");
  Serial.println(curve_count);
  char *str = new char[strlen(figure)+1];
  strcpy(str, figure);
  char **figures = new char*[curve_count];
  int counter = 0;
  if (curve_count > 1) {
    char *figures_token = strtok(str, ";");
    while (figures_token) {
      figures[counter] = figures_token;
      counter++;
      figures_token = strtok(NULL, ";");
    }
  } else {
    figures[0] = str;
  }
  Serial.println("Seperate succeed");
  for (int i=0;i<curve_count;i++) {
    Serial.print("Curve #");
    Serial.print(i);
    Serial.print(' ');
    char *figure = figures[i];
    int vector_count = 1;
    for (char *ptr=figure;*ptr!='\0';ptr++) {
      if (*ptr == '+') {
        vector_count++;
      }
    }
    vector2 *vectors = new vector2[vector_count];
    Serial.print("VectorCount: ");
    Serial.println(vector_count);
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
      vectors[counter] = {i1*0.02+offset.x*0.02, i2*0.02+offset.y*0.02};
      counter++;
      t = strtok(NULL, "+");
    }
    Serial.print("Applying curve #");
    Serial.println(i);
    apply_coords(vectors[0]);
    if (pen_state == HANG) {
      delay(500);
      pen(DROP);
    }
    draw_bezier_curve(vectors, vector_count);
    if (hang) {
      pen(HANG);
    }
    delete vectors;
  }
  delete figures;
  delete str;
  delay(500);
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
  lcd.print("CONNECTING");
  while (Serial.available() <= 0) {
    delay(10);
  }
  int rsize = Serial.read();
  char *rstr = new char[rsize+1];
  memset(rstr, 0, rsize+1);
  Serial.readBytes(rstr, rsize);
  if (strcmp(rstr, "REQ_C MIWRITER") == 0) {
    lcd.setCursor(0, 1);
    lcd.print("CONNECTED ");
  }
  delete rstr;
  while (true) {
    while (Serial.available() <= 0) {
      delay(10);
    }
    int size = Serial.read();
    char *str = new char[size+1];
    memset(str, 0, size+1);
    Serial.readBytes(str, size);
    if (str[0] == 'C') {
      draw_figurestr(characters[str[1]]);
    }
    delete str;
  }
}
#else
void loop() {
  lcd.clear();
  lcd.print("Hand-Writer v1.0");
  lcd.setCursor(0, 1);
  lcd.print("test");
  delay(2000);
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
