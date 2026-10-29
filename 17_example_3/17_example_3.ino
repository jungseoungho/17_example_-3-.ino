#include <Servo.h>

// Arduino pin assignment

#define PIN_IR    0         // IR sensor at Pin A0
#define PIN_LED   9
#define PIN_SERVO 10

// TODO: 서보 실험에서 얻은 본인의 값으로 교체할 것
#define _DUTY_MIN 1000  // servo full clock-wise position (0 degree)
#define _DUTY_NEU 1500  // servo neutral position (90 degree)
#define _DUTY_MAX 2000  // servo full counter-clockwise position (180 degree)

#define _DIST_MIN  100.0   // minimum distance 100mm
#define _DIST_MAX  250.0   // maximum distance 250mm

#define EMA_ALPHA  0.3      // for EMA Filter (0 < alpha <= 1, 작을수록 더 부드러움)

#define LOOP_INTERVAL 25   // Loop Interval (unit: msec), 20 이상

Servo myservo;
unsigned long last_loop_time;   // unit: msec

float dist_prev = _DIST_MIN;
float dist_ema = _DIST_MIN;

void setup()
{
  pinMode(PIN_LED, OUTPUT);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(57600);   
}

void loop()
{
  unsigned long time_curr = millis();
  int duty;
  float a_value, dist_raw;

  // wait until next event time
  if (time_curr < (last_loop_time + LOOP_INTERVAL))
    return;
  last_loop_time += LOOP_INTERVAL;

  a_value = analogRead(PIN_IR);

  // a_value == 9 이면 0으로 나누게 되므로 방지
  if (a_value <= 9.0)
    dist_raw = _DIST_MAX + 1.0;   // 범위 밖으로 처리
  else
    dist_raw = ((6762.0 / (a_value - 9.0)) - 4.0) * 10.0 - 60.0;

  // range filter (_DIST_MIN ~ _DIST_MAX)
  if (dist_raw < _DIST_MIN || dist_raw > _DIST_MAX) {
    dist_raw = dist_prev;          // 범위 밖이면 이전 값 유지
    digitalWrite(PIN_LED, HIGH);    // 범위 밖: LED off
  } else {
    dist_prev = dist_raw;          // 범위 안이면 값 갱신
    digitalWrite(PIN_LED, LOW);   // 범위 안: LED on
  }

  // EMA filter
  dist_ema = EMA_ALPHA * dist_raw + (1.0 - EMA_ALPHA) * dist_ema;

  // map() equivalent: 100~250mm -> _DUTY_MIN~_DUTY_MAX
  duty = (int)(_DUTY_MIN +
               (dist_ema - _DIST_MIN) * (float)(_DUTY_MAX - _DUTY_MIN) / (_DIST_MAX - _DIST_MIN));

  myservo.writeMicroseconds(duty);

  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println("");
}
