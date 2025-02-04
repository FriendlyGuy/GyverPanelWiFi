
byte hue;

// *********** снегопад 2.0 ***********

void snowRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_SNOW;
    FastLED.clear();  // очистить
  }

  // сдвигаем всё вниз
  shiftDown();

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2) == 0 && (random8(0, map8(255 - effectScaleParam[MC_SNOW],5,15)) == 0)) {
      CRGB color = CRGB(effectBrightness,effectBrightness,effectBrightness); /*0xE0FFFF*/
      if (color.r > 0x20 && random8(0, 4) == 0) color = color - CRGB(0x10, 0x10, 0x10);
      drawPixelXY(x, HEIGHT - 1, color);
    } else {
      drawPixelXY(x, HEIGHT - 1, 0x000000);
    }
  }
}

// ------------- ПЕЙНТБОЛ -------------

uint8_t USE_SEGMENTS_PAINTBALL = 0;
uint8_t BorderWidth = 0;
uint8_t dir_mx, seg_num, seg_size, seg_offset, seg_offset_x, seg_offset_y;
int16_t idx;

void lightBallsRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_PAINTBALL;
    FastLED.clear();  // очистить
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                         // Размер квадратного сегмента (высота и ширина равны)
    seg_offset = ((dir_mx == 0 ? WIDTH : HEIGHT) - seg_size * seg_num) / (seg_num + 1); // смещение от края матрицы и между сегментами    
    BorderWidth = 0;
    USE_SEGMENTS_PAINTBALL = effectScaleParam2[MC_PAINTBALL];
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = map(effectBrightness, 32,255, 65,91);
  uint8_t actualBrightness = map(effectBrightness, 32,255, 125,250);
  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  // The color of each point shifts over time, each at a different speed.
  uint32_t ms = millis();

  byte  cnt = map8(255-effectScaleParam[MC_PAINTBALL],1,4);  // 1..4 шариков
  float spd = (map8(255-effectSpeed, 50, 100) / 100.0) / (USE_SEGMENTS_PAINTBALL != 0 ? 1 : (float)seg_num);

  // Отрисовка режима происходит на максимальной скорости. Знеачение effectSpeed влияет на параметр BPM функции beatsin8
  // The easiest way to construct this is to multiply a floating point BPM value (e.g. 120.3) by 256, (e.g. resulting in 30796 in this case), and pass that as the 16-bit BPM argument.
  byte m1 = ( 91.0 * spd) + 0.51;
  byte m2 = (109.0 * spd) + 0.51;
  byte m3 = ( 73.0 * spd) + 0.51;
  byte m4 = (123.0 * spd) + 0.51;

  // Для неквадратных - вычленяем квадратные сегменты, которые равномерно распределяем по ширине / высоте матрицы 

  if (USE_SEGMENTS_PAINTBALL != 0) {
    uint8_t  i = beatsin8(m1, 0, seg_size - BorderWidth - 1);
    uint8_t  j = beatsin8(m2, 0, seg_size - BorderWidth - 1);
    uint8_t  k = beatsin8(m3, 0, seg_size - BorderWidth - 1);
    uint8_t  m = beatsin8(m4, 0, seg_size - BorderWidth - 1);

    uint8_t d1 = ms / 29;
    uint8_t d2 = ms / 41;
    uint8_t d3 = ms / 73;
    uint8_t d4 = ms / 97;
    
    for (uint8_t ii = 0; ii < seg_num; ii++) {
      delay(0); // Для предотвращения ESP8266 Watchdog Timer      
      uint8_t cx = dir_mx == 0 ? (seg_offset * (ii + 1) + seg_size * ii) : 0;
      uint8_t cy = dir_mx == 0 ? 0 : (seg_offset * (ii + 1) + seg_size * ii);
      uint8_t color_shift = ii * 50;
      if (cnt <= 1) { idx = XY(i+cx, j+cy); leds[idx] += CHSV( color_shift + d1, 200, actualBrightness); }
      if (cnt <= 2) { idx = XY(j+cx, k+cy); leds[idx] += CHSV( color_shift + d2, 200, actualBrightness); }
      if (cnt <= 3) { idx = XY(k+cx, m+cy); leds[idx] += CHSV( color_shift + d3, 200, actualBrightness); }
      if (cnt <= 4) { idx = XY(m+cx, i+cy); leds[idx] += CHSV( color_shift + d4, 200, actualBrightness); }
      
      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые 
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      byte fade_step = map8(effectBrightness, 1, 15);
      for (byte i2 = cy; i2 < cy + seg_size; i2++) { 
        fadePixel(cx + BorderWidth, i2, fade_step);
        fadePixel(cx + seg_size - BorderWidth - 1, i2, fade_step);
      }
    }
  }
  else 
  {
    uint8_t  i = beatsin8(m1, BorderWidth, WIDTH - BorderWidth - 1);
    uint8_t  j = beatsin8(m1, BorderWidth, HEIGHT - BorderWidth - 1);
    uint8_t  k = beatsin8(m3, BorderWidth, WIDTH - BorderWidth - 1);
    uint8_t  m = beatsin8(m4, BorderWidth, HEIGHT - BorderWidth - 1);
    
    if (cnt <= 1) { idx = XY(i, j); leds[idx] += CHSV( ms / 29, 200, actualBrightness); }
    if (cnt <= 2) { idx = XY(k, j); leds[idx] += CHSV( ms / 41, 200, actualBrightness); }
    if (cnt <= 3) { idx = XY(k, m); leds[idx] += CHSV( ms / 73, 200, actualBrightness); }
    if (cnt <= 4) { idx = XY(i, m); leds[idx] += CHSV( ms / 97, 200, actualBrightness); }
  
    if (WIDTH == HEIGHT) {
      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые 
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      byte fade_step = map8(effectBrightness, 1, 15);
      for (byte i = 0; i < HEIGHT; i++) { 
        fadePixel(0, i, fade_step);
        fadePixel(WIDTH-1, i, fade_step);
      }
    } 
  }
}

// ------------- ВОДОВОРОТ -------------

uint8_t USE_SEGMENTS_SWIRL = 0;

void swirlRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_SWIRL;
    FastLED.clear();  // очистить
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                         // Размер квадратного сегмента (высота и ширина равны)
    seg_offset = ((dir_mx == 0 ? WIDTH : HEIGHT) - seg_size * seg_num) / (seg_num + 1); // смещение от края матрицы и между сегментами    
    BorderWidth = seg_num == 1 ? 0 : 1;
    USE_SEGMENTS_SWIRL = effectScaleParam2[MC_SWIRL];
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.

  uint8_t blurAmount = map(effectBrightness, 32,255, 65,91);
  uint8_t actualBrightness = map(effectBrightness, 32,255, 125,250);
  blur2d( leds, WIDTH, HEIGHT, blurAmount);

  uint32_t ms = millis();  
  int16_t idx;

  float spd = (map8(255-effectSpeed, 50, 100) / 100.0) / (USE_SEGMENTS_PAINTBALL != 0 ? 1 : (float)seg_num);

  // Отрисовка режима происходит на максимальной скорости. Знеачение effectSpeed влияет на параметр BPM функции beatsin8
  // The easiest way to construct this is to multiply a floating point BPM value (e.g. 120.3) by 256, (e.g. resulting in 30796 in this case), and pass that as the 16-bit BPM argument.
  byte m1 = (41.0 * spd) + 0.51;
  byte m2 = (27.0 * spd) + 0.51;

  if (USE_SEGMENTS_SWIRL != 0) {
    // Use two out-of-sync sine waves
    uint8_t  i = beatsin8(m1, 0, seg_size - BorderWidth - 1);
    uint8_t  j = beatsin8(m2, 0, seg_size - BorderWidth - 1);

    // Also calculate some reflections
    uint8_t ni = (seg_size-1)-i;
    uint8_t nj = (seg_size-1)-j;

    uint8_t d1 = ms / 11;
    uint8_t d2 = ms / 13;
    uint8_t d3 = ms / 17;
    uint8_t d4 = ms / 29;
    uint8_t d5 = ms / 37;
    uint8_t d6 = ms / 41;
    
    for (uint8_t ii = 0; ii < seg_num; ii++) {
      delay(0); // Для предотвращения ESP8266 Watchdog Timer      
      uint8_t cx = dir_mx == 0 ? (seg_offset * (ii + 1) + seg_size * ii) : 0;
      uint8_t cy = dir_mx == 0 ? 0 : (seg_offset * (ii + 1) + seg_size * ii);
      uint8_t color_shift = ii * 50;
    
      // The color of each point shifts over time, each at a different speed.
      idx = XY( i+cx, j+cy); leds[idx] += CHSV( color_shift + d1, 200, actualBrightness);
      idx = XY(ni+cx,nj+cy); leds[idx] += CHSV( color_shift + d2, 200, actualBrightness);
      idx = XY( i+cx,nj+cy); leds[idx] += CHSV( color_shift + d3, 200, actualBrightness);
      idx = XY(ni+cx, j+cy); leds[idx] += CHSV( color_shift + d4, 200, actualBrightness);
      idx = XY( j+cx, i+cy); leds[idx] += CHSV( color_shift + d5, 200, actualBrightness);
      idx = XY(nj+cx,ni+cy); leds[idx] += CHSV( color_shift + d6, 200, actualBrightness);
      
      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые 
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      byte fade_step = map8(effectBrightness, 1, 15);
      for (byte i2 = cy; i2 < cy + seg_size; i2++) { 
        fadePixel(cx, i2, fade_step);
        fadePixel(cx + BorderWidth, i2, fade_step);
        fadePixel(cx + seg_size - 1, i2, fade_step);
        fadePixel(cx + seg_size - BorderWidth - 1, i2, fade_step);
      }
    }
  } 
  else 
  {
    // Use two out-of-sync sine waves
    uint8_t  i = beatsin8(m1, BorderWidth, WIDTH - BorderWidth - 1);
    uint8_t  j = beatsin8(m2, BorderWidth, HEIGHT - BorderWidth - 1);

    // Also calculate some reflections
    uint8_t ni = (WIDTH-1)-i;
    uint8_t nj = (HEIGHT-1)-j;

    // The color of each point shifts over time, each at a different speed.
    idx = XY( i, j); leds[idx] += CHSV( ms / 11, 200, actualBrightness);
    idx = XY(ni,nj); leds[idx] += CHSV( ms / 13, 200, actualBrightness);
    idx = XY( i,nj); leds[idx] += CHSV( ms / 17, 200, actualBrightness);
    idx = XY(ni, j); leds[idx] += CHSV( ms / 29, 200, actualBrightness);
    
    if (HEIGHT == WIDTH) {
      // для квадратных матриц - 6 точек создают более красивую картину
      idx = XY( j, i); leds[idx] += CHSV( ms / 37, 200, actualBrightness);
      idx = XY(nj,ni); leds[idx] += CHSV( ms / 41, 200, actualBrightness);
      
      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые 
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      byte fade_step = map8(effectBrightness, 1, 15);
      for (byte i = 0; i < HEIGHT; i++) { 
        fadePixel(0, i, fade_step);
        fadePixel(WIDTH-1, i, fade_step);
      }
    }  
  }
}

// Эта функция в FastLED объявлена как forward;
// линкуется с библиотекой FastLed, которая использует её для определения индекса светодиода в массиве leds[]
// ври вызове функций типа blur2d() и т.п.
uint16_t XY(uint8_t x, uint8_t y) { 
  return getPixelNumber(x, y);
}

// ***************************** БЛУДНЫЙ КУБИК *****************************

#define RANDOM_COLOR 1    // случайный цвет при отскоке

int coordB[2];
int8_t vectorB[2];
CRGB ballColor;
int8_t ballSize;

void ballRoutine() {

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  
  if (loadingFlag) {
    for (byte i = 0; i < 2; i++) {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random8(8, 20);
      ballColor = CHSV(random8(0, 9) * 28, 255, effectBrightness);
    }
    // modeCode = MC_BALL;
    loadingFlag = false;
  }

  ballSize = map8(effectScaleParam[MC_BALL],2, max(min(WIDTH,HEIGHT) / 3, 2));

  for (byte i = 0; i < 2; i++) {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0) {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random8(0, 9) * 28, 255, effectBrightness);
      //vectorB[i] += random8(0, 6) - 3;
    }
  }

  if (coordB[0] > (WIDTH - ballSize) * 10) {
    coordB[0] = (WIDTH - ballSize) * 10;
    vectorB[0] = -vectorB[0];
    if (RANDOM_COLOR) ballColor = CHSV(random8(0, 9) * 28, 255, effectBrightness);
    //vectorB[0] += random8(0, 6) - 3;
  }

  if (coordB[1] > (HEIGHT - ballSize) * 10) {
    coordB[1] = (HEIGHT - ballSize) * 10;
    vectorB[1] = -vectorB[1];
    if (RANDOM_COLOR) ballColor = CHSV(random8(0, 9) * 28, 255, effectBrightness);
    //vectorB[1] += random8(0, 6) - 3;
  }

  FastLED.clear();

  for (byte i = 0; i < ballSize; i++)
    for (byte j = 0; j < ballSize; j++)
      leds[getPixelNumber(coordB[0] / 10 + i, coordB[1] / 10 + j)] = ballColor;
}

// ***************************** РАДУГА *****************************

byte rainbow_type = 0;

void rainbowRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_RAINBOW;
    rainbow_type = (specialTextEffectParam >= 0) ? specialTextEffectParam : effectScaleParam2[MC_RAINBOW];
    // Если авто - генерировать один из типов - 1-Вертикальная радуга, 2-Горизонтальная радуга, 3-Диагональная радуга, 4-Вращающаяся радуга
    if (rainbow_type == 0 || rainbow_type > 4) {
      rainbow_type = random8(1,4);
    }     
    FastLED.clear();  // очистить
  }

  switch (rainbow_type) {
    case 1:  rainbowVertical(); break;
    case 2:  rainbowHorizontal(); break;
    case 3:  rainbowDiagonal(); break;
    default: rainbowRotate(); break;
  }
}

// *********** радуга дигональная ***********

void rainbowDiagonal() {
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  hue += 2;
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      float dx = (WIDTH>=HEIGHT)
         ? (float)(WIDTH / HEIGHT * x + y)
         : (float)(HEIGHT / WIDTH * y + x);
      CRGB thisColor = CHSV((byte)(hue + dx * (float)(255 / maxDim)), 255, effectBrightness);
      drawPixelXY(x, y, thisColor); 
    }
  }
}

// *********** радуга горизонтальная ***********

void rainbowHorizontal() {
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  hue += 2;
  for (byte j = 0; j < HEIGHT; j++) {
    CHSV thisColor = CHSV((byte)(hue + j * map8(effectScaleParam[MC_RAINBOW],1,WIDTH)), 255, effectBrightness);
    for (byte i = 0; i < WIDTH; i++)
      drawPixelXY(i, j, thisColor);
  }
}

// *********** радуга вертикальная ***********

void rainbowVertical() {
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  hue += 2;
  for (byte i = 0; i < WIDTH; i++) {
    CHSV thisColor = CHSV((byte)(hue + i * map8(effectScaleParam[MC_RAINBOW],1,HEIGHT)), 255, effectBrightness);
    for (byte j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, thisColor);
  }
}

// *********** радуга вращающаяся ***********

void rainbowRotate() {
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / WIDTH));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / HEIGHT));

  byte   lineStartHue = ms / 65536;
  int8_t yHueDelta8   = yHueDelta32 / 32768;
  int8_t xHueDelta8   = xHueDelta32 / 32768;
  
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  for( byte y = 0; y < HEIGHT; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < WIDTH; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue, 255, effectBrightness);
    }
  }
}

// ---------------------------------------- ЦВЕТА ------------------------------------------

void colorsRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_COLORS;
    FastLED.clear();  // очистить
  }
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  hue += map8(effectScaleParam[MC_COLORS],1,10);
  CHSV hueColor = CHSV(hue, 255, effectBrightness);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = hueColor;
  }
}

// ---------------------------------------- ЦИКЛОН ------------------------------------------

int16_t cycle_x, cycle_y; // могут уходить в минус при смене направления
uint8_t move_dir, fade_divider, inc_cnt, USE_SEGMENTS_CYCLON;

void cyclonRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_CYCLON;
    USE_SEGMENTS_CYCLON = effectScaleParam2[MC_CYCLON];
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                                                       // 0 - сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);                                           // вычисляем количество сегментов, умещающихся на матрице, в режиме без сигментов ширина одной полоски будет равна кол-ву сегментов
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                                                               // Размер квадратного сегмента (высота и ширина равны)
    seg_offset_y = USE_SEGMENTS_CYCLON == 1 ? (dir_mx == 1 ? HEIGHT - seg_size * seg_num : 0) / 2 : 0;     // смещение от низа/верха матрицы
    seg_offset_x = USE_SEGMENTS_CYCLON == 1 ? (dir_mx == 0 ? WIDTH - seg_size * seg_num : 0) / 2 : 0;      // смещение от левого/правого края матрицы
    hue = 0;
    cycle_x = USE_SEGMENTS_CYCLON == 1 ? (seg_offset_x + seg_size - 1) : WIDTH - 1; // начало - от правого края к левому
    cycle_y = USE_SEGMENTS_CYCLON == 1 ?  seg_offset_y : 0;
    move_dir = 1;
    fade_divider = 0;
    inc_cnt = NUM_LEDS / 312;
    if (inc_cnt == 0) inc_cnt = 1;
    FastLED.clear();  // очистить
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  byte actualBrightness = map(effectBrightness, 32,255, 125,250);
  
  // Использовать отрисовку по сегментам
  // Если сегменты не используется - ширина одной полоски - кол-во сегментов
  for (byte i=0; i < seg_num; i++) {
    for (byte k=0; k < inc_cnt; k++) { 
      if (USE_SEGMENTS_CYCLON == 1) {
        if (cycle_y + k - seg_offset_y >= seg_size) continue;
        idx = dir_mx == 0
           ? getPixelNumber(cycle_x + i * seg_size, cycle_y + k)
           : getPixelNumber(cycle_x, cycle_y + i * seg_size + k);
      } else {
        if (cycle_y + k  >= HEIGHT) continue;
        idx = getPixelNumber(cycle_x + i, cycle_y + k);
      }
      if (idx >= 0 && idx < NUM_LEDS) 
          leds[idx] = CHSV(hue + k + (USE_SEGMENTS_CYCLON == 1 ? i * 85 : 0), 255, actualBrightness);              
    }
  }  

  hue += inc_cnt;
  
  // Затухание - не на каждый цикл, а регулируется параметром эффекта
  byte fader_param = map8(255 - effectScaleParam[MC_CYCLON],0,5);
  fade_divider++;
  if (fade_divider > fader_param) {
    fade_divider = 0;
    fader(5);
  }

  cycle_y += inc_cnt;

  if (USE_SEGMENTS_CYCLON) {
    
    if (cycle_y - seg_offset_y >= seg_size) {
      cycle_y = seg_offset_y;
  
      if (move_dir == 0) {
        // Слева направо
        cycle_x++;     
        if (cycle_x - seg_offset_x >= seg_size) {
            move_dir = 1;
            cycle_x = seg_size - 1 + seg_offset_x;
        }
      } else {
        // Справа налево
        cycle_x--;     
        if (cycle_x < seg_offset_x) {
            move_dir = 0;
            cycle_x = seg_offset_x;
        }
      }    
    }
    
  } else {
    
    if (cycle_y >= HEIGHT) {
      cycle_y = 0;
  
      if (move_dir == 0) {
        // Слева направо
        cycle_x += seg_num;     
        if (cycle_x >= WIDTH) {
            move_dir = 1;
            cycle_x = WIDTH - 1;
        }
      } else {
        // Справа налева
        cycle_x -= seg_num;     
        if (cycle_x < 0) {
            move_dir = 0;
            cycle_x = 0;
        }
      }    
    }
    
  }
  
}

// ********************** огонь **********************

#define SPARKLES 1        // вылетающие угольки вкл выкл

unsigned char matrixValue[8][16];
unsigned char line[WIDTH];
int pcnt = 0;

//these values are substracetd from the generated values to give a shape to the animation
const unsigned char valueMask[8][16] PROGMEM = {
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
const unsigned char hueMask[8][16] PROGMEM = {
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};

void fireRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_FIRE;
    FastLED.clear();
    generateLine();
    memset(matrixValue, 0, sizeof(matrixValue));
  }

  if (pcnt >= 100) {
    shiftFireUp();
    generateLine();
    pcnt = 0;
  }

  drawFrame(pcnt);

  pcnt += 30;
}

// Randomly generate the next line (matrix row)

void generateLine() {
  for (uint8_t x = 0; x < WIDTH; x++) {
    line[x] = random8(64, 255);
  }
}

//shift all values in the matrix up one row

void shiftFireUp() {
  for (uint8_t y = HEIGHT - 1; y > 0; y--) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint8_t newX = x;
      if (x > 15) newX = x%16;
      if (y > 7) continue;
      matrixValue[y][newX] = matrixValue[y - 1][newX];
    }
  }

  for (uint8_t x = 0; x < WIDTH; x++) {
    uint8_t newX = x;
    if (x > 15) newX = x%16;
    matrixValue[0][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(int pcnt) {
  int nextv;

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  //each row interpolates with the one before it
  for (unsigned char y = HEIGHT - 1; y > 0; y--) {
    for (unsigned char x = 0; x < WIDTH; x++) {
      uint8_t newX = x;
      if (x > 15) newX = x%16;
      if (y < 8) {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
            - pgm_read_byte(&(valueMask[y][newX]));

        
        CRGB color = CHSV(
                       map8(effectScaleParam[MC_FIRE],0,230) + pgm_read_byte(&(hueMask[y][newX])), // H
                       255, // S
                       (uint8_t)max(0, nextv) // V
                     );
                     
        CRGB color2 = color.nscale8_video(effectBrightness);

        leds[getPixelNumber(x, y)] = color2;
      } else if (y == 8 && SPARKLES) {
        if (random8(0, 20) == 0 && getPixColorXY(x, y - 1) != 0) 
          drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else 
          drawPixelXY(x, y, 0);
      } else if (SPARKLES) {

        // старая версия для яркости
        if (getPixColorXY(x, y - 1) > 0)
          drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else 
          drawPixelXY(x, y, 0);
      }
    }
  }

  //first row interpolates with the "next" line
  for (unsigned char x = 0; x < WIDTH; x++) {
    uint8_t newX = x;
    if (x > 15) newX = x%16;
    CRGB color = CHSV(
                   map8(effectScaleParam[MC_FIRE],0,230) + pgm_read_byte(&(hueMask[0][newX])), // H
                   255,           // S
                   (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0) // V
                 );
    CRGB color2 = color.nscale8_video(effectBrightness);
                 
    //leds[getPixelNumber(newX, 0)] = color2; // На форуме пишут что это ошибка - вместо newX должно быть x, иначе
    leds[getPixelNumber(x, 0)] = color2;      // на матрицах шире 16 столбцов нижний правый угол неработает
  }
}

// **************** МАТРИЦА *****************

void matrixRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_MATRIX;
    FastLED.clear();
  }
  
  byte     effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  uint32_t cut_out = HEIGHT < 10 ? 0x40 : 0x20; // на 0x004000 хвосты мматрицы короткие (4 точки), на 0x002000 - длиннее (8 точек)

  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    CRGB thisColor = getPixColorXY(x, HEIGHT - 1);
    if (thisColor.g == 0) {
      leds[getPixelNumber(x, HEIGHT - 1)] = random8(0, map8(255 - effectScaleParam[MC_MATRIX],5,15)) == 0 ? CRGB(0, effectBrightness, 0) : CRGB(0,0,0);
    } else if (thisColor.g < cut_out)
      drawPixelXY(x, HEIGHT - 1, 0);
    else
      drawPixelXY(x, HEIGHT - 1, thisColor - CRGB(cut_out, cut_out, cut_out));
  }

  // сдвигаем всё вниз
  shiftDown();
}


// ********************************* ШАРИКИ *********************************

#define BALLS_AMOUNT_MAX 6 // максимальное количество "шариков"
#define CLEAR_PATH 1       // очищать путь
#define BALL_TRACK 1       // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP 70      // длина хвоста шарика (чем больше цифра, тем хвост короче)

int8_t BALLS_AMOUNT;
int coord[BALLS_AMOUNT_MAX][2];
int8_t vector[BALLS_AMOUNT_MAX][2];
byte ballColors[BALLS_AMOUNT_MAX];

void ballsRoutine() {
  if (loadingFlag) {
    // modeCode = MC_BALLS;
    loadingFlag = false;
    FastLED.clear();
    
    // Текущее количество шариков из настроек
    BALLS_AMOUNT = map8(effectScaleParam[MC_BALLS],3,6); 
    
    for (byte j = 0; j < BALLS_AMOUNT; j++) {
      int sign;

      // забиваем случайными данными
      coord[j][0] = WIDTH / 2 * 10;
      random8(0, 2) ? sign = 1 : sign = -1;
      vector[j][0] = random8(4, 15) * sign;
      coord[j][1] = HEIGHT / 2 * 10;
      random8(0, 2) ? sign = 1 : sign = -1;
      vector[j][1] = random8(4, 15) * sign;
      ballColors[j] = random8(0, 255);
    }
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  if (!BALL_TRACK)    // если режим БЕЗ следов шариков
    FastLED.clear();  // очистить
  else {              // режим со следами
    fader(map8(effectBrightness, 4, TRACK_STEP));
  }

  // движение шариков
  for (byte j = 0; j < BALLS_AMOUNT; j++) {

    // движение шариков
    for (byte i = 0; i < 2; i++) {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0) {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0] > (WIDTH - 1) * 10) {
      coord[j][0] = (WIDTH - 1) * 10;
      vector[j][0] = -vector[j][0];
    }
    if (coord[j][1] > (HEIGHT - 1) * 10) {
      coord[j][1] = (HEIGHT - 1) * 10;
      vector[j][1] = -vector[j][1];
    }
    leds[getPixelNumber(coord[j][0] / 10, coord[j][1] / 10)] =  CHSV(ballColors[j], 255, effectBrightness);
  }
}

// ********************* ЗВЕЗДОПАД ******************

#define TAIL_STEP  80     // длина хвоста кометы (чем больше цифра, тем хвост короче)
#define SATURATION 150    // насыщенность кометы (от 0 до 255)

int8_t STAR_DENSE;     // плотность комет 30..90

void starfallRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_STARFALL;
    FastLED.clear();  // очистить
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  STAR_DENSE = map8(effectScaleParam[MC_SPARKLES],30,90);
  
  // заполняем головами комет левую и верхнюю линию
  for (byte i = 4; i < HEIGHT; i++) {
    if (getPixColorXY(0, i) == 0
        && (random8(0, STAR_DENSE) == 0)
        && getPixColorXY(0, i + 1) == 0
        && getPixColorXY(0, i - 1) == 0)
      leds[getPixelNumber(0, i)] = CHSV(random8(0, 200), SATURATION, effectBrightness);
  }
  
  for (byte i = 0; i < WIDTH-4; i++) {
    if (getPixColorXY(i, HEIGHT - 1) == 0
        && (random8(0, map8(effectScaleParam[MC_STARFALL],10,120)) == 0)
        && getPixColorXY(i + 1, HEIGHT - 1) == 0
        && getPixColorXY(i - 1, HEIGHT - 1) == 0)
      leds[getPixelNumber(i, HEIGHT - 1)] = CHSV(random8(0, 200), SATURATION, effectBrightness);
  }

  // сдвигаем по диагонали
  shiftDiag();

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (byte i = 4; i < HEIGHT; i++) {
    fadePixel(0, i, TAIL_STEP);
  }
  for (byte i = 0; i < WIDTH-4; i++) {
    fadePixel(i, HEIGHT - 1, TAIL_STEP);
  }

}

// *********************  КОНФЕТТИ ******************

#define BRIGHT_STEP 70    // шаг уменьшения яркости

void sparklesRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_SPARKLES;
    FastLED.clear();  // очистить
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  for (byte i = 0; i < map8(effectScaleParam[MC_SPARKLES],1,25); i++) {
    byte x = random8(0, WIDTH);
    byte y = random8(0, HEIGHT);
    if (getPixColorXY(x, y) == 0)
      leds[getPixelNumber(x, y)] = CHSV(random8(0, 255), 255, effectBrightness);
  }

  fader(map8(effectBrightness, 4, BRIGHT_STEP));
}

// ----------------------------- СВЕТЛЯКИ ------------------------------

#define LIGHTERS_AM 100
int lightersPos[2][LIGHTERS_AM];
int8_t lightersSpeed[2][LIGHTERS_AM];
byte lightersColor[LIGHTERS_AM];
byte loopCounter;

int angle[LIGHTERS_AM];
int speedV[LIGHTERS_AM];
int8_t angleSpeed[LIGHTERS_AM];

void lightersRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    // modeCode = MC_LIGHTERS;
    randomSeed(millis());
    for (byte i = 0; i < LIGHTERS_AM; i++) {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);
      lightersSpeed[0][i] = random(-10, 10);
      lightersSpeed[1][i] = random(-10, 10);
      lightersColor[i] = random(0, 255);
    }
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  FastLED.clear();

  if (++loopCounter > 20) loopCounter = 0;

  for (byte i = 0; i < map8(effectScaleParam[MC_LIGHTERS],5,150); i++) {
    if (loopCounter == 0) {     // меняем скорость каждые 20 отрисовок
      lightersSpeed[0][i] += random(-3, 4);
      lightersSpeed[1][i] += random(-3, 4);
      lightersSpeed[0][i] = constrain(lightersSpeed[0][i], -20, 20);
      lightersSpeed[1][i] = constrain(lightersSpeed[1][i], -20, 20);
    }

    lightersPos[0][i] += lightersSpeed[0][i];
    lightersPos[1][i] += lightersSpeed[1][i];

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0) {
      lightersPos[1][i] = 0;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    if (lightersPos[1][i] >= (HEIGHT - 1) * 10) {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, CHSV(lightersColor[i], 255, effectBrightness));
  }
}

// ********************* БУДИЛЬНИК-РАССВЕТ *********************

int8_t row, col;                   // Для эффекта спирали  - точка "глолвы" змейки, бегающей по спирали (первая змейка для круговой спирали)
int8_t row2, col2;                 // Для эффекта спирали  - точка "глолвы" змейки, бегающей по спирали (вторая змейка для плоской спирали)
int8_t dir, dir2;                  // Для эффекта спирали на плоскости - направление движениия змейки: 0 - вниз; 1 - влево; 2 - вверх; 3 - вправо; 
int8_t range[4], range2[4];        // Для эффекта спирали на плоскости - границы разворачивания спирали; 
uint16_t tail[8], tail2[8];        // Для эффекта спирали на плоскости - позиции хвоста змейки. HiByte = x, LoByte=y
CHSV tailColor;                    // Цвет последней точки "хвоста" змейки. Этот же цвет используется для предварительной заливки всей матрицы
CHSV tailColor2;                   // Предварительная заливка нужна для корректного отображения часов поверх специальных эффектов будильника
boolean firstRowFlag;              // Флаг начала самого первого ряда первого кадра, чтобы не рисовать "хвост" змейки в предыдущем кадре, которого не было.
byte dawnBrightness;               // Текущая яркость будильника "рассвет"
byte tailBrightnessStep;           // Шаг приращения яркости будильника "рассвет"
byte dawnColorIdx;                 // Индекс в массиве цвета "заливки" матрицы будильника "рассвет" (голова змейки)
byte dawnColorPrevIdx;             // Предыдущий индекс - нужен для корректного цвета отрисовки "хвоста" змейки, 
                                   // когда голова начинает новый кадр внизу матрицы, а хвост - вверху от предыдущего кадра
byte step_cnt;                     // Номер шага эффекта, чтобы определить какой длины "хвост" у змейки

// "Рассвет" - от красного к желтому - белому - голубому с плавным увеличением яркости;
// Яркость меняется по таймеру - на каждое срабатывание таймера - +1 к яркости.
// Диапазон изменения яркости - от MIN_DAWN_BRIGHT до MAX_DAWN_BRIGHT (количество шагов)
// Цветовой тон матрицы меняется каждые 16 шагов яркости 255 / 16 -> дает 16 индексов в массиве цветов
// Время таймера увеличения яркости - время рассвета DAWN_NINUTES на количество шагов приращения яркости
byte dawnColorHue[16]  PROGMEM = {0, 16, 28, 36, 44, 52, 57, 62, 64, 66, 66, 64, 62, 60, 128, 128};              // Цвет заполнения - HUE змейки 1
byte dawnColorSat[16]  PROGMEM = {255, 250, 245, 235, 225, 210, 200, 185, 170, 155, 130, 105, 80, 50, 25, 80};   // Цвет заполнения - SAT змейки 1
byte dawnColorHue2[16] PROGMEM = {0, 16, 28, 36, 44, 52, 57, 62, 64, 66, 66, 64, 62, 60, 128, 128};              // Цвет заполнения - HUE змейки 2
byte dawnColorSat2[16] PROGMEM = {255, 250, 245, 235, 225, 210, 200, 185, 170, 155, 130, 105, 80, 50, 25, 80};   // Цвет заполнения - SAT змейки 2

#define MIN_DAWN_BRIGHT   2        // Минимальное значение яркости будильника (с чего начинается)
#define MAX_DAWN_BRIGHT   255      // Максимальное значение яркости будильника (чем заканчивается)
byte DAWN_NINUTES = 20;            // Продолжительность рассыета в минутах

void dawnProcedure() {

  if (loadingFlag) {
    dawnBrightness = MIN_DAWN_BRIGHT;
    // modeCode = MC_DAWN_ALARM;
    
    FastLED.clear();  // очистить
    FastLED.setBrightness(dawnBrightness);        

    if (realDawnDuration <= 0 || realDawnDuration > dawnDuration) realDawnDuration = dawnDuration;
    long interval = realDawnDuration * 60000L / (MAX_DAWN_BRIGHT - MIN_DAWN_BRIGHT);
    dawnTimer.setInterval(interval);
  }

  // Пришло время увеличить яркость рассвета?
  if (dawnTimer.isReady() && dawnBrightness < 255) {
    dawnBrightness++;
    FastLED.setBrightness(dawnBrightness);
  }

  byte effect = isAlarming ? alarmEffect : MC_DAWN_ALARM;
  if (effect == MC_DAWN_ALARM) {
    // Если устройство лампа (DEVICE_TYPE == 0) - матрица свернута в "трубу" - рассвет - огонек, бегущий вкруговую по спирали
    // Если устройство плоская матрица в рамке (DEVICE_TYPE == 1) - рассвет - огонек, бегущий по спирали от центра матрицы к краям на плоскости
    effect = DEVICE_TYPE == 0 ? MC_DAWN_ALARM_SPIRAL : MC_DAWN_ALARM_SQUARE;
  }

  // Если эффект "Лампа" и цвет - черный (остался от "выключено" - выбрать цвет лампы из сохраненных эффектов "Цветная лампа"
  if (effect == MC_FILL_COLOR && globalColor == 0) {
     set_globalColor(getColorInt(CHSV(getEffectSpeed(MC_FILL_COLOR), effectScaleParam[MC_FILL_COLOR], 255)));
  }
  if (effect == MC_FILL_COLOR && globalColor == 0) {
     set_globalColor(0xFFFFFF);
  }

  // Сформировать изображение эффекта
  processEffect(effect);
  
  // Сбрасывать флаг нужно ПОСЛЕ того как инициализированы: И процедура рассвета И применяемый эффект,
  // используемый в качестве рассвета
  loadingFlag = false;
}
  
// "Рассвет" по спирали, для ламп на круговой матрице (свернутой в трубу)
void dawnLampSpiral() {

  if (loadingFlag) {
    row = 0, col = 0;
    
    dawnBrightness = MIN_DAWN_BRIGHT; 
    tailBrightnessStep = 16;
    firstRowFlag = true;
    dawnColorIdx = 0;
    dawnColorPrevIdx = 0;
    
    tailColor = CHSV(0, 255, 255 - 8 * tailBrightnessStep); 
  }

  boolean flag = true;
  int8_t x=col, y=row;
  
  if (!firstRowFlag) {
    fillAll(tailColor);
  }

  byte tail_len = min(8, WIDTH - 1);  
  for (byte i=0; i<tail_len; i++) {
    x--;
    if (x < 0) { x = WIDTH - 1; y--; }
    if (y < 0) {
      y = HEIGHT - 1;
      flag = false;
      if (firstRowFlag) break;
    }

    byte idx = y > row ? dawnColorPrevIdx : dawnColorIdx;
    byte dawnHue = pgm_read_byte(&(dawnColorHue[idx]));
    byte dawnSat = pgm_read_byte(&(dawnColorSat[idx]));
        
    tailColor = CHSV(dawnHue, dawnSat, 255 - i * tailBrightnessStep); 
    drawPixelXY(x,y, tailColor);  
  }
  
  if (flag) {
    firstRowFlag = false;
    dawnColorPrevIdx = dawnColorIdx;
  }
  if (dawnBrightness == 255 && tailBrightnessStep > 8) tailBrightnessStep -= 2;
  
  col++;
  if (col >= WIDTH) {
    col = 0; row++;
  }
  
  if (row >= HEIGHT) row = 0;  

  if (col == 0 && row == 0) {
    // Кол-во элементов массива - 16; Шагов яркости - 255; Изменение индекса каждые 16 шагов яркости. 
    dawnColorIdx = dawnBrightness >> 4;  
  }
}

// "Рассвет" по спирали на плоскости, для плоских матриц
void dawnLampSquare() {

  if (loadingFlag) {
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                         // Размер квадратного сегмента (высота и ширина равны)
    seg_offset = ((dir_mx == 0 ? WIDTH : HEIGHT) - seg_size * seg_num) / (seg_num + 1); // смещение от края матрицы и между сегментами        

    SetStartPos();
    
    dawnBrightness = MIN_DAWN_BRIGHT; 
    tailBrightnessStep = 16;
    dawnColorIdx = 0;
    step_cnt = 0;

    memset(tail, 0, sizeof(uint16_t) * 8);
    memset(tail2, 0, sizeof(uint16_t) * 8);
    
    tailColor = CHSV(0, 255, 255 - 8 * tailBrightnessStep); 
  }
  
  int8_t x=col, y=row;
  int8_t x2=col2, y2=row2;

  fillAll(tailColor);
  
  step_cnt++;
  
  for (byte i=7; i>0; i--) {
    tail[i]  = tail[i-1];
    tail2[i] = tail2[i-1];
  }
  tail[0]  = (uint)((int)x <<8 | (int)y);
  tail2[0] = (uint)((int)x2<<8 | (int)y2);

  byte dawnHue  = pgm_read_byte(&(dawnColorHue[dawnColorIdx]));
  byte dawnSat  = pgm_read_byte(&(dawnColorSat[dawnColorIdx]));
  byte dawnHue2 = pgm_read_byte(&(dawnColorHue2[dawnColorIdx]));
  byte dawnSat2 = pgm_read_byte(&(dawnColorSat2[dawnColorIdx]));

  for (byte i=0; i < 8; i++) {
    
    tailColor  = CHSV(dawnHue, dawnSat, 255 - i * tailBrightnessStep); 
    tailColor2 = CHSV(dawnHue2, dawnSat2, 255 - i * tailBrightnessStep); 

    if (i<=step_cnt) {
      x  =  tail[i] >>8;  
      y  = tail[i]  & 0xff;
      x2 =  tail2[i]>>8;  
      y2 = tail2[i] & 0xff;
      for (byte n=0; n < seg_num; n++) {
        uint8_t cx = dir_mx == 0 ? (seg_offset * (n + 1) + seg_size * n) : 0;
        uint8_t cy = dir_mx == 0 ? 0 : (seg_offset * (n + 1) + seg_size * n);
        drawPixelXY(x + cx,  y + cy,  tailColor);
        drawPixelXY(x2 + cx, y2 + cy, tailColor2);  
      }
    }
  }
  
  if (dawnBrightness == 255 && tailBrightnessStep > 8) tailBrightnessStep -= 2;

  switch(dir) {
    case 0: // вниз;
      row--;
      if (row <= range[dir]) {
        range[dir] = row - 2;
        dir++;
      }
      break;
    case 1: // влево;
      col--;
      if (col <= range[dir]) {
        range[dir] = col - 2;
        dir++;
      }
      break;
    case 2: // вверх;
      row++;
      if (row >= range[dir]) {
        range[dir] = row + 2;
        dir++;
      }
      break;
    case 3: // вправо;
      col++;
      if (col >= range[dir]) {
        range[dir] = col + 2;
        dir = 0;        
      }
      break;
  }
  
  switch(dir2) {
    case 0: // вниз;
      row2--;
      if (row2 <= range2[dir2]) {
        range2[dir2] = row2 - 2;
        dir2++;
      }
      break;
    case 1: // влево;
      col2--;
      if (col2 <= range2[dir2]) {
        range2[dir2] = col2 - 2;
        dir2++;
      }
      break;
    case 2: // вверх;
      row2++;
      if (row2 >= range2[dir2]) {
        range2[dir2] = row2 + 2;
        dir2++;
      }
      break;
    case 3: // вправо;
      col2++;
      if (col2 >= range2[dir2]) {
        range2[dir2] = col2 + 2;
        dir2 = 0;        
      }
      break;
  }
  
  bool out  = (col  < 0 || col  >= seg_size) && (row  < 0 || row  >= seg_size);
  bool out2 = (col2 < 0 || col2 >= seg_size) && (row2 < 0 || row2 >= seg_size);
  if (out && out2) {
    // Кол-во элементов массива - 16; Шагов яркости - 255; Изменение индекса каждые 16 шагов яркости. 
    dawnColorIdx = dawnBrightness >> 4;  
    SetStartPos();
    step_cnt = 0;
  }
}

void SetStartPos() {
  if (seg_size % 2 == 1)
  {
    col = seg_size / 2 + 1;
    col2 = col;
    row = seg_size / 2 + 1;
    row2 = row;
  } else {
    col = seg_size / 2 - 1;
    col2 = seg_size - col - 1;
    row = seg_size / 2 - 1;
    row2 = seg_size - row - 1;
  }
  
  dir = 2; dir2 = 0;
  
  // 0 - вниз; 1 - влево; 2 - вверх; 3 - вправо;
  range[0] = row-2; range[1] = col-2; range[2] = row+2; range[3] = col+2;
  range2[0] = row2-2; range2[1] = col2-2; range2[2] = row2+2; range2[3] = col2+2;
}

// ******************* ЛАМПА ********************

void fillColorProcedure() {
  if (loadingFlag) {
    // modeCode = MC_FILL_COLOR;
    loadingFlag = false;
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  byte bright = isAlarming && !isAlarmStopped 
    ? dawnBrightness
    : (specialMode ? specialBrightness : effectBrightness);

  if (globalColor == 0) {
    fillAll(CRGB::Black);
  } else {
    CRGB color = globalColor;
    color.nscale8_video(bright);
    fillAll(color);
  }
}

// ******************* МЕРЦАНИЕ ********************

uint32_t xf,yf,v_time,hue_time,hxy;

// Play with the values of the variables below and see what kinds of effects they
// have!  More octaves will make things slower.

// how many octaves to use for the brightness and hue functions
uint8_t octaves=1;
uint8_t hue_octaves=3;

// the 'distance' between points on the x and y axis
int xscale=57771;
int yscale=57771;

// the 'distance' between x/y points for the hue noise
int hue_scale=1;

// how fast we move through time & hue noise
int time_speed=1111;
int hue_speed=1;

// adjust these values to move along the x or y axis between frames
int x_speed = (WIDTH > HEIGHT ? 1111 : 331);
int y_speed = (WIDTH > HEIGHT ? 331 : 1111);

void flickerRoutine() {
  if (loadingFlag) {
    // modeCode = MC_FLICKER;
    loadingFlag = false;

    hxy = (uint32_t)((uint32_t)random16() << 16) + (uint32_t)random16();
    xf = (uint32_t)((uint32_t)random16() << 16) + (uint32_t)random16();
    yf = (uint32_t)((uint32_t)random16() << 16) + (uint32_t)random16();
    v_time = (uint32_t)((uint32_t)random16() << 16) + (uint32_t)random16();
    hue_time = (uint32_t)((uint32_t)random16() << 16) + (uint32_t)random16();    
  }

  // byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  // fill the led array 2/16-bit noise values
  fill_2dnoise16(leds, WIDTH, HEIGHT, (MATRIX_TYPE == 0),
                octaves, xf, xscale, yf, yscale, v_time,
                hue_octaves, hxy, hue_scale, hxy, hue_scale, hue_time, 
                false);

  // adjust the intra-frame time values
  hue_speed  = map8(255-effectSpeed, 1, 10);

  xf += x_speed;
  yf += y_speed;

  v_time += time_speed;
  hue_time += hue_speed;
}

// ******************* PACIFICA ********************

//////////////////////////////////////////////////////////////////////////
//
// In this animation, there are four "layers" of waves of light.  
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then 
// another filter is applied that adds "whitecaps" of brightness where the 
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent 
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };

void pacificaRoutine()
{
  if (loadingFlag) {
    // modeCode = MC_PACIFICA;
    loadingFlag = false;
  }

  // byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  
  sLastms = ms;
  
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;

  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

// ********************** SHADOWS ***********************

void shadowsRoutine() {
  if (loadingFlag) {
    // modeCode = MC_SHADOWS;
    loadingFlag = false;
  }

  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, map8(bri8, map(effectBrightness, 32, 255, 32,125), map(effectBrightness, 32,255, 125,250))); 
    
    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS-1) - pixelnumber;
    
    nblend( leds[pixelnumber], newcolor, 64);
  }
}

// ***************************** ПАЛИТРА *****************************

#define BLOCK_SIZE 4       // Размер квадратика палитры
#define FADE_IN_STEPS 16    // За сколько шагов плашка появляется на экране    
#define FADE_OUT_STEPS 32  // За сколько шагов плашка убирается с экрана    
#define BLOCK_ON_START 5   // Сколько блоков сразу появлять в начале эффекта

byte num_x, num_y, off_x, off_y;

byte palette_h[WIDTH / BLOCK_SIZE][HEIGHT / BLOCK_SIZE]; // Н in CHSV
byte palette_s[WIDTH / BLOCK_SIZE][HEIGHT / BLOCK_SIZE]; // S in CHSV
byte block_sta[WIDTH / BLOCK_SIZE][HEIGHT / BLOCK_SIZE]; // Block state: // 0 - появление; 1 - исчезновение; 2 - пауза перед появлением 3 - пауза перед удалением
byte block_dur[WIDTH / BLOCK_SIZE][HEIGHT / BLOCK_SIZE]; // время паузы блока

void paletteRoutine() {

  if (loadingFlag) {
    // modeCode = MC_PALETTE;
    loadingFlag = false;

    num_x = WIDTH / BLOCK_SIZE;
    num_y = HEIGHT / BLOCK_SIZE;
    off_x = (WIDTH - BLOCK_SIZE * num_x) / 2;
    off_y = (HEIGHT - BLOCK_SIZE * num_y) / 2;

    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице

    // Для всех блоков определить состояние - "ожидание появления
    for (byte c = 0; c < num_x; c++) {
      for (byte r = 0; r < num_y; r++) {
        block_sta[c][r] = 2;                // Состояние - пауза перед появлением
        block_dur[c][r] = random8(25,125);  // Длительность паузы
      }
    }

    // Для некоторого количества начальных - установить "За шаг до появления"
    // При первом же проходе состояние переключится на "появление"
    for (byte i = 0; i < BLOCK_ON_START * seg_num; i++) {
      byte idx = random8(0, num_x * num_y - 1);
      byte r = idx / BLOCK_SIZE;
      byte c = idx % BLOCK_SIZE;
      block_dur[c][r] = 1;                  // Счетчик до начала появления
    }
    FastLED.clear();
  }
  
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  for (byte c = 0; c < num_x; c++) {
    byte block_x = off_x + c * BLOCK_SIZE;
    for (byte r = 0; r < num_y; r++) {    
      
      byte block_y = off_y + r * BLOCK_SIZE;
      byte h = palette_h[c][r];      
      byte s = palette_s[c][r];

      // Проверить состояние блока
      if (block_sta[c][r] > 1) {
        
        // Одна из пауз (2 или 3) - пауза перед появлением или перед исчезновением
        // Уменьшить время паузы. Если стало 0 - переключить с паузы на появление / исчезновение
         block_dur[c][r] -= 1;
         if (block_dur[c][r] == 0) {
           block_sta[c][r] -= 2;     // 3->1 - исчезать; 2->0 появлять за указанное количество шагов
           if (block_sta[c][r] == 0) {
             block_dur[c][r] = FADE_IN_STEPS;    // Количество шагов появления блока
             palette_h[c][r] = random8(0,255);   // Цвет нового блока
             palette_s[c][r] = random8(112,254); // Насыщенность цвета нового блока
           } else { 
             block_dur[c][r] = FADE_OUT_STEPS;  // Кол-во шагов убирания блока
           }  
         }

      }
      
      if (block_sta[c][r] < 2) {

        // В процессе появления или исчезновения (0 или 1)
        // Выполнить один шаг появления / исчезновения блока
        byte fade_dir = block_sta[c][r]; // 0 - появляться, 1 - исчезать
        byte fade_step = block_dur[c][r];

        // Яркость блока
        byte bri = fade_dir == 0
           ? map(fade_step, 0,FADE_IN_STEPS,  0,effectBrightness)
           : map(fade_step, 0,FADE_OUT_STEPS, effectBrightness,0);

        // Нарисовать блок   
        for (byte i=0; i<BLOCK_SIZE; i++) {        
          for (byte j=0; j<BLOCK_SIZE; j++) {
            
            //byte k = fade_dir == 0 ? (2 * i*j) : (2 * (BLOCK_SIZE * BLOCK_SIZE - i*j));
            //byte bri2 = (bri > k ? bri - k : 0);
            CHSV color = CHSV(h, s, bri); // bri2

            byte xx = block_x + j;
            byte yy = block_y + BLOCK_SIZE - i - 1;
            if (xx < WIDTH && yy < HEIGHT) {
              uint16_t idx = getPixelNumber(xx, yy);
              leds[idx] = color;
            }
          }
        }

        // Шаг появления - обработан
        block_dur[c][r] -= 1;

        // Весь процесс появления / исчезновения выполнен?
        // Сменить статус блока
        if (block_dur[c][r] == 0) {
           // Появление / исчезновение закончено
           block_sta[c][r] = block_sta[c][r] == 0 ? 3 : 2; // вкл паузу перед исчезновением после появления или паузу перед появлением после исчезновения
           block_dur[c][r] = random8(25,125);              // Длительность паузы (циклов обращения палитры)
        }        
      }      
    }
  }

}

// ****************************** ANALYZER *****************************

// цвета высоты полос спектра.
#define COLOR1    HUE_GREEN
#define COLOR2    HUE_YELLOW
#define COLOR3    HUE_ORANGE
#define COLOR4    HUE_RED
#define MAX_COLOR HUE_RED // цвет точек максимума

// анимация
#define SMOOTH 0.3        // плавность движения столбиков (0 - 1)

// точки максимума
#define MAX_DOTS 1        // включить/выключить отрисовку точек максимума (1 вкл, 0 выкл)
#define FALL_DELAY 50     // скорость падения точек максимума (задержка, миллисекунды)
#define FALL_PAUSE 700    // пауза перед падением точек максимума, миллисекунды

#define MAX_LEVEL (HEIGHT + HEIGHT / 4)
#define SIN_WIDTH (WIDTH / 8)

unsigned long gainTimer, fallTimer;
unsigned long timeLevel[WIDTH];
byte  maxValue;
byte  posOffset[WIDTH];   // Массив данных для отображения на матрице
int   maxLevel[WIDTH];
byte  posLevel_old[WIDTH];
boolean fallFlag;

byte st = 0;
byte phase = 0;          // фаза эффекта
    
// -------------------------------------------------------------------------------------


void analyzerRoutine() {

  if (loadingFlag) {
    // modeCode = MC_ANALYZER;
    loadingFlag = false;
    for (int i = 0; i < WIDTH; i++) {
      maxLevel[i] = 0;
      posLevel_old[i] = 0;
    }
    st = 0;
    phase = 0;
    FastLED.clear();
  }
  
  if (phase == 0) {
    // Движение волны слева направо
    for (int i = 0; i < WIDTH; i++) {
      posOffset[i] = (i < st || i >= st + SIN_WIDTH - (SIN_WIDTH / 4))
        ? 0
        : map8(sin8(map(i, st,st + SIN_WIDTH, 0,255)), 1, HEIGHT + HEIGHT / 2);
    }
  } else 

  if (phase == 2) {
    // Движение волны справа налево
    for (int i = 0; i < WIDTH; i++) {
        posOffset[i] = (i < WIDTH - st || i > WIDTH - st + SIN_WIDTH)
          ? 0
          : map8(sin8(map(i, WIDTH-st, WIDTH - st + SIN_WIDTH, 0, 255)), 1, HEIGHT + HEIGHT / 2);
    }
  } else

  if (phase == 1 || phase == 3) {
    // Пауза, даем "отстояться" пикам
    for (int i = 0; i < WIDTH; i++) {
      posOffset[i] = 0;
    }    
  } else
  
  if (phase >= 4) {
    // Случайные двиижения - "музыка"
    for (int i = 0; i < WIDTH; i++) {
      posOffset[i] = random8(1,MAX_LEVEL);    
    }
  }

  st++;
  if (st>=WIDTH && phase < 4) {    
    phase++;
    st = phase % 2 == 1 ? WIDTH / 2 : 0;
  }
  
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  
  maxValue = 0;
  FastLED.clear();  // очистить матрицу
  
  for (byte pos = 0; pos < WIDTH; pos++) {    // для кажого столбца матрицы
    int posLevel = posOffset[pos];

    // найти максимум из пачки тонов
    if (posLevel > maxValue) maxValue = posLevel;

    // фильтрация длины столбиков, для их плавного движения
    posLevel = posLevel * SMOOTH + posLevel_old[pos] * (1 - SMOOTH);
    posLevel_old[pos] = posLevel;

    // преобразовать значение величины спектра в диапазон 0..HEIGHT с учётом настроек
    posLevel = constrain(posLevel, 1, HEIGHT - 1);

    if (posLevel > 0) {
      for (int j = 0; j < posLevel; j++) {
        CHSV color;
        if      (j < map( 5, 0,16, 0,HEIGHT)) color = CHSV(COLOR1, 255, effectBrightness);
        else if (j < map(10, 0,16, 0,HEIGHT)) color = CHSV(COLOR2, 255, effectBrightness);
        else if (j < map(13, 0,16, 0,HEIGHT)) color = CHSV(COLOR3, 255, effectBrightness);
        else if (j < map(15, 0,16, 0,HEIGHT)) color = CHSV(COLOR4, 255, effectBrightness);

        drawPixelXY(pos, j, color);
      }
    }

    if (posLevel > 0 && posLevel > maxLevel[pos]) {    // если для этой полосы есть максимум, который больше предыдущего
      maxLevel[pos] = posLevel;                        // запомнить его
      timeLevel[pos] = millis();                       // запомнить время
    }

    // если точка максимума выше нуля (или равна ему) - включить пиксель
    if (maxLevel[pos] >= 0 && MAX_DOTS) {
      drawPixelXY(pos, maxLevel[pos], CHSV(MAX_COLOR, 255, effectBrightness));
    }

    if (fallFlag) {                                           // если падаем на шаг
      if ((long)millis() - timeLevel[pos] > FALL_PAUSE) {     // если максимум держался на своей высоте дольше FALL_PAUSE
        if (maxLevel[pos] >= 0) maxLevel[pos]--;              // уменьшить высоту точки на 1
        // внимание! Принимает минимальное значение -1 !
      }
    }
  }

  fallFlag = 0;                                 // сбросить флаг падения
  if (millis() - fallTimer > FALL_DELAY) {      // если настало время следующего падения
    fallFlag = 1;                               // поднять флаг
    fallTimer = millis();
  }
  
}

// ****************************** СИНУСЫ *****************************

void prizmataRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    dir_mx = WIDTH >= HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    // modeCode = MC_PRIZMATA;
  }
  
  EVERY_N_MILLIS(33) {
     hue++;
  }
  
  FastLED.clear();

  // Отрисовка режима происходит на максимальной скорости. Знеачение effectSpeed влияет на параметр BPM функции beatsin8
  byte spd = map8(255-effectSpeed, 12, 64);   
  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  if (dir_mx == 0) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint8_t y = beatsin8(spd + x, 0, HEIGHT-1);
      drawPixelXY(x, y, ColorFromPalette(RainbowColors_p, x * 7 + hue, effectBrightness));
    }
  } else {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      uint8_t x = beatsin8(spd + y, 0, WIDTH-1);
      drawPixelXY(x, y, ColorFromPalette(RainbowColors_p, x * 7 + hue, effectBrightness));
    }
  }
}

// *************************** ВЫШИВАНКА **************************

int8_t count = 0;

byte flip = 0;
byte generation = 0;

const TProgmemRGBPalette16 MunchColors_p FL_PROGMEM =
{
    0xFF0000, 0x000000, 0xAB5500, 0x000000,
    0xABAB00, 0x000000, 0x00FF00, 0x000000,
    0x00AB55, 0x000000, 0x0000FF, 0x000000,
    0x5500AB, 0x000000, 0xAB0055, 0x000000
};

void munchRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    //modeCode = MC_MUNCH;
    generation = 0;
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                         // Размер квадратного сегмента (высота и ширина равны)
    seg_offset = ((dir_mx == 0 ? WIDTH : HEIGHT) - seg_size * seg_num) / (seg_num + 1); // смещение от края матрицы и между сегментами    
    dir = 1;
    FastLED.clear();
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  for (byte x = 0; x < seg_size; x++) {
    for (byte y = 0; y < seg_size; y++) {
      for (byte n = 0; n < seg_num; n++) {
        CRGB color = ((x ^ y ^ flip) < count ? ColorFromPalette(MunchColors_p, ((x ^ y) << 4) + generation, effectBrightness) : CRGB::Black);
        if (dir_mx == 0)
          drawPixelXY(seg_offset + x + (n * seg_size), y, color);
        else   
          drawPixelXY(x, seg_offset + y + (n * seg_size), color);
      }
    }
  }

  count += dir;

  if (count <= 0 || count >= seg_size) {
    dir = dir < 0 ? 1 : -1;
  }

  if (count <= 0) {
    if (flip == 0)
      flip = seg_size - 1; 
    else
      flip = 0;
  }

  generation++;
}

// *************************** ДОЖДЬ **************************

CRGB rainColor = CRGB(60,80,90);
CRGB lightningColor = CRGB(72,72,80);
CRGBPalette16 rain_p( CRGB::Black, rainColor);
CRGBPalette16 rainClouds_p( CRGB::Black, CRGB(15,24,24), CRGB(9,15,15), CRGB::Black );

#define NUM_LAYERSMAX 2
uint8_t noise3d[NUM_LAYERSMAX][WIDTH][HEIGHT];

void rain(byte backgroundDepth, byte spawnFreq, byte tailLength, bool splashes, bool clouds, bool storm) {
  
  static uint16_t noiseX = random16();
  static uint16_t noiseY = random16();
  static uint16_t noiseZ = random16();

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);
  
  fadeToBlackBy( leds, NUM_LEDS, 255-tailLength);

  // Loop for each column individually
  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Move each dot down one cell
    for (uint8_t i = 0; i < HEIGHT; i++) {
      if (noise3d[0][x][i] >= backgroundDepth) {  // Don't move empty cells
        if (i > 0) noise3d[0][x][wrapY(i-1)] = noise3d[0][x][i];
        noise3d[0][x][i] = 0;
      }
    }

    // Step 2.  Randomly spawn new dots at top
    if (random8() < spawnFreq) {
      noise3d[0][x][HEIGHT-1] = random(backgroundDepth, effectBrightness);
    }

    // Step 3. Map from tempMatrix cells to LED colors
    for (uint8_t y = 0; y < HEIGHT; y++) {
      if (noise3d[0][x][y] >= backgroundDepth) {  // Don't write out empty cells
        leds[XY(x,y)] = ColorFromPalette(rain_p, noise3d[0][x][y], effectBrightness);
      }
    }

    // Step 4. Add splash if called for
    if (splashes) {
      // FIXME, this is broken
      byte j = line[x];
      byte v = noise3d[0][x][0];

      if (j >= backgroundDepth) {
        leds[XY(wrapX(x-2),0)] = ColorFromPalette(rain_p, j/3, effectBrightness);
        leds[XY(wrapX(x+2),0)] = ColorFromPalette(rain_p, j/3, effectBrightness);
        line[x] = 0;   // Reset splash
      }

      if (v >= backgroundDepth) {
        leds[XY(wrapX(x-1),1)] = ColorFromPalette(rain_p, v/2, effectBrightness);
        leds[XY(wrapX(x+1),1)] = ColorFromPalette(rain_p, v/2, effectBrightness);
        line[x] = v; // Prep splash for next frame
      }
    }

    // Step 5. Add lightning if called for
    if (storm && random16() < 72) {
      
      // uint8_t lightning[WIDTH][HEIGHT];
      // ESP32 does not like static arrays  https://github.com/espressif/arduino-esp32/issues/2567
      uint8_t *lightning = (uint8_t *) malloc(WIDTH * HEIGHT);
      
      if (lightning != NULL) { 
        lightning[scale8(random8(), WIDTH-1) + (HEIGHT-1) * WIDTH] = 255;  // Random starting location
        for(uint8_t ly = HEIGHT-1; ly > 1; ly--) {
          for (uint8_t lx = 1; lx < WIDTH-1; lx++) {
            if (lightning[lx + ly * WIDTH] == 255) {
              lightning[lx + ly * WIDTH] = 0;
              uint8_t dir = random8(4);
              switch (dir) {
                case 0:
                  leds[XY(lx+1,ly-1)] = lightningColor;
                  lightning[(lx+1) + (ly-1) * WIDTH] = 255; // move down and right
                break;
                case 1:
                  leds[XY(lx,ly-1)] = CRGB(128,128,128);   // я без понятия, почему у верхней молнии один оттенок, а у остальных - другой
                  lightning[lx + (ly-1) * WIDTH] = 255;    // move down
                break;
                case 2:
                  leds[XY(lx-1,ly-1)] = CRGB(128,128,128);
                  lightning[(lx-1) + (ly-1) * WIDTH] = 255; // move down and left
                break;
                case 3:
                  leds[XY(lx-1,ly-1)] = CRGB(128,128,128);
                  lightning[(lx-1) + (ly-1) * WIDTH] = 255; // fork down and left
                  leds[XY(lx-1,ly-1)] = CRGB(128,128,128);
                  lightning[(lx+1) + (ly-1) * WIDTH] = 255; // fork down and right
                break;
              }
            }
          }
        }
        free(lightning);
      } else {
        DEBUGLN("lightning malloc failed"); 
      }
    }

    // Step 6. Add clouds if called for
    if (clouds) {
      uint16_t noiseScale = 250;  // A value of 1 will be so zoomed in, you'll mostly see solid colors. A value of 4011 will be very zoomed out and shimmery
      const uint8_t cloudHeight = HEIGHT * 0.2 + 1; // это уже 20% c лишеним, но на высоких матрицах будет чуть меньше

      // This is the array that we keep our computed noise values in
      //static uint8_t noise[WIDTH][cloudHeight];
      static uint8_t *noise = (uint8_t *) malloc(WIDTH * cloudHeight);
      
      if (noise != NULL) {      
        int xoffset = noiseScale * x + hue;  
        for(uint8_t z = 0; z < cloudHeight; z++) {
          int yoffset = noiseScale * z - hue;
          uint8_t dataSmoothing = 192;
          uint8_t noiseData = qsub8(inoise8(noiseX + xoffset,noiseY + yoffset,noiseZ),16);
          noiseData = qadd8(noiseData,scale8(noiseData,39));
          noise[x * cloudHeight + z] = scale8( noise[x * cloudHeight + z], dataSmoothing) + scale8( noiseData, 256 - dataSmoothing);
          nblend(leds[XY(x,HEIGHT-z-1)], ColorFromPalette(rainClouds_p, noise[x * cloudHeight + z], effectBrightness), (cloudHeight-z)*(250/cloudHeight));
        }
      } else { 
        DEBUGLN("noise malloc failed"); 
      } 
      noiseZ++;
    }
  }
}


void rainRoutine()
{
  if (loadingFlag) {
    loadingFlag = false;
    //modeCode = MC_RAIN;
  }

  byte intensity = beatsin8(map8(effectScaleParam[MC_RAIN],2,6), 4, 60);
  
  // ( Depth of dots, frequency of new dots, length of tails, splashes, clouds, ligthening )
  if (intensity <= 35) 
    // Lightweight
    rain(60, intensity, 10, true, true, false);
  else
    // Stormy
    rain(0, intensity, 10, true, true, true);
}

// ************************* ВОДОПАД ************************

#define COOLINGNEW 32
#define SPARKINGNEW 80
extern const TProgmemRGBPalette16 WaterfallColors_p FL_PROGMEM = {0x000000, 0x060707, 0x101110, 0x151717, 0x1C1D22, 0x242A28, 0x363B3A, 0x313634, 0x505552, 0x6B6C70, 0x98A4A1, 0xC1C2C1, 0xCACECF, 0xCDDEDD, 0xDEDFE0, 0xB2BAB9};

void waterfallRoutine() {

  if (loadingFlag) {
    loadingFlag = false;
    //modeCode = MC_WATERFALL;
  }

  byte effectBrightness = getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);

  for (uint8_t x = 0; x < WIDTH; x++) {

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < HEIGHT; i++) {
      noise3d[0][x][i] = qsub8(noise3d[0][x][i], random8(0, ((COOLINGNEW * 10) / HEIGHT) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = HEIGHT - 1; k >= 2; k--) {
      noise3d[0][x][k] = (noise3d[0][x][k - 1] + noise3d[0][x][k - 2] + noise3d[0][x][k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKINGNEW) {
      int y = random8(2);
      noise3d[0][x][y] = qadd8(noise3d[0][x][y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < HEIGHT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8(noise3d[0][x][j], 240);
      leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(WaterfallColors_p, colorindex, effectBrightness);
    }
  }
}

// ********************** ОГОНЬ-2 (КАМИН) *********************

void fire2Routine()
{
  if (loadingFlag) {
    loadingFlag = false;
    //modeCode = MC_FIRE2;
  }
  
#if HEIGHT/6 > 6
  #define FIRE_BASE 6
#else
  #define FIRE_BASE HEIGHT/6+1
#endif
  
  // COOLING: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.  
  byte cooling = map8(effectSpeed, 70, 100);     
  
  // SPARKING: What chance (out of 255) is there that a new spark will be lit?
  // Higher chance = more roaring fire.  Lower chance = more flickery fire.
  byte sparking = map8(effectScaleParam[MC_FIRE2], 90, 150);
  
  // SMOOTHING; How much blending should be done between frames
  // Lower = more blending and smoother flames. Higher = less blending and flickery flames
  const uint8_t fireSmoothing = 80;
  
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  byte effectBrightness = map8(getBrightnessCalculated(globalBrightness, effectContrast[thisMode]), 32,128);

  // Loop for each column individually
  for (uint8_t x = 0; x < WIDTH; x++) {
    
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++) {
      noise3d[0][x][i] = qsub8(noise3d[0][x][i], random(0, ((cooling * 10) / HEIGHT) + 2));
    }

    // Step 2.  Heat from  cell drifts 'up' and diffuses a little
    for (uint8_t k = HEIGHT; k > 1; k--) {
      noise3d[0][x][wrapY(k)] = (noise3d[0][x][k - 1] + noise3d[0][x][wrapY(k - 2)] + noise3d[0][x][wrapY(k - 2)]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking) {
      uint8_t j = random8(FIRE_BASE);
      noise3d[0][x][j] = qadd8(noise3d[0][x][j], random(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    // Blend new data with previous frame. Average data between neighbouring pixels
    for (uint8_t y = 0; y < HEIGHT; y++)
      nblend(leds[XY(x,y)], ColorFromPalette(HeatColors_p, ((noise3d[0][x][y]*0.7) + (noise3d[0][wrapX(x+1)][y]*0.3)), effectBrightness), fireSmoothing);
  }
}

// ************************** СТРЕЛКИ *************************
int8_t  arrow_x[4], arrow_y[4], stop_x[4], stop_y[4];
byte    arrow_direction;            // 0x01 - слева направо; 0x02 - снизу вверх; 0х04 - справа налево; 0х08 - сверху вниз
byte    arrow_mode, arrow_mode_orig;// 0 - по очереди все варианты
                                    // 1 - по очереди от края до края экрана; 
                                    // 2 - одновременно по горизонтали навстречу к ентру, затем одновременно по вертикали навстречу к центру
                                    // 3 - одновременно все к центру
                                    // 4 - по два (горизонталь / вертикаль) все от своего края к противоположному, стрелки смещены от центра на 1/3
                                    // 5 - одновременно все от своего края к противоположному, стрелки смещены от центра на 1/3
bool    arrow_complete, arrow_change_mode;
byte    arrow_hue[4];
byte    arrow_play_mode_count[6];        // Сколько раз проигрывать полностью каждый режим если вариант 0 - текущий счетчик
byte    arrow_play_mode_count_orig[6];   // Сколько раз проигрывать полностью каждый режим если вариант 0 - исходные настройки

void arrowsRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    //modeCode = MC_ARROWS;
    FastLED.clear();
    arrow_complete = false;
    arrow_mode_orig = (specialTextEffectParam >= 0) ? specialTextEffectParam : effectScaleParam2[MC_ARROWS];
    
    arrow_mode = (arrow_mode_orig == 0 || arrow_mode_orig > 5) ? random8(1,5) : arrow_mode_orig;
    arrow_play_mode_count_orig[0] = 0;
    arrow_play_mode_count_orig[1] = 4;  // 4 фазы - все стрелки показаны по кругу один раз - переходить к следующему ->
    arrow_play_mode_count_orig[2] = 4;  // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[3] = 4;  // 1 фаза - все к центру (1) повторить по 4 раза -> 4
    arrow_play_mode_count_orig[4] = 4;  // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[5] = 4;  // 1 фаза - все сразу (1) повторить по 4 раза -> 4
    for (byte i=0; i<6; i++) {
      arrow_play_mode_count[i] = arrow_play_mode_count_orig[i];
    }
    arrowSetupForMode(arrow_mode, true);
  }
  
  byte effectBrightness = map8(getBrightnessCalculated(globalBrightness, effectContrast[thisMode]), 32,255);  

  fader(65);
  CHSV color;
  
  // движение стрелки - cлева направо
  if ((arrow_direction & 0x01) > 0) {
    color = CHSV(arrow_hue[0], 255, effectBrightness);
    for (int8_t x = 0; x <= 4; x++) {
      for (int8_t y = 0; y <= x; y++) {    
        if (arrow_x[0] - x >= 0 && arrow_x[0] - x <= stop_x[0]) { 
          CHSV clr = (x < 4 || (x == 4 && y < 2)) ? color : CHSV(0,0,0);
          drawPixelXY(arrow_x[0] - x, arrow_y[0] - y, clr);
          drawPixelXY(arrow_x[0] - x, arrow_y[0] + y, clr);
        }
      }    
    }
    arrow_x[0]++;
  }

  // движение стрелки - cнизу вверх
  if ((arrow_direction & 0x02) > 0) {
    color = CHSV(arrow_hue[1], 255, effectBrightness);
    for (int8_t y = 0; y <= 4; y++) {
      for (int8_t x = 0; x <= y; x++) {    
        if (arrow_y[1] - y >= 0 && arrow_y[1] - y <= stop_y[1]) { 
          CHSV clr = (y < 4 || (y == 4 && x < 2)) ? color : CHSV(0,0,0);
          drawPixelXY(arrow_x[1] - x, arrow_y[1] - y, clr);
          drawPixelXY(arrow_x[1] + x, arrow_y[1] - y, clr);
        }
      }    
    }
    arrow_y[1]++;
  }

  // движение стрелки - cправа налево
  if ((arrow_direction & 0x04) > 0) {
    color = CHSV(arrow_hue[2], 255, effectBrightness);
    for (int8_t x = 0; x <= 4; x++) {
      for (int8_t y = 0; y <= x; y++) {    
        if (arrow_x[2] + x >= stop_x[2] && arrow_x[2] + x < WIDTH) { 
          CHSV clr = (x < 4 || (x == 4 && y < 2)) ? color : CHSV(0,0,0);
          drawPixelXY(arrow_x[2] + x, arrow_y[2] - y, clr);
          drawPixelXY(arrow_x[2] + x, arrow_y[2] + y, clr);
        }
      }    
    }
    arrow_x[2]--;
  }

  // движение стрелки - cверху вниз
  if ((arrow_direction & 0x08) > 0) {
    color = CHSV(arrow_hue[3], 255, effectBrightness);
    for (int8_t y = 0; y <= 4; y++) {
      for (int8_t x = 0; x <= y; x++) {    
        if (arrow_y[3] + y >= stop_y[3] && arrow_y[3] + y < HEIGHT) { 
          CHSV clr = (y < 4 || (y == 4 && x < 2)) ? color : CHSV(0,0,0);
          drawPixelXY(arrow_x[3] - x, arrow_y[3] + y, clr);
          drawPixelXY(arrow_x[3] + x, arrow_y[3] + y, clr);
        }
      }    
    }
    arrow_y[3]--;
  }

  // Проверка завершения движения стрелки, переход к следующей фазе или режиму
  
  switch (arrow_mode) {

    case 1:
      // Последовательно - слева-направо -> снизу вверх -> справа налево -> сверху вниз и далее по циклу
      // В каждый сомент времени сктивна только одна стрелка, если она дошла до края - переключиться на следующую и задать ее начальные координаты
      arrow_complete = false;
      switch (arrow_direction) {
        case 1: arrow_complete = arrow_x[0] > stop_x[0]; break;
        case 2: arrow_complete = arrow_y[1] > stop_y[1]; break;
        case 4: arrow_complete = arrow_x[2] < stop_x[2]; break;
        case 8: arrow_complete = arrow_y[3] < stop_y[3]; break;
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = (arrow_direction << 1) & 0x0F;
        if (arrow_direction == 0) arrow_direction = 1;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[1]--;
          if (arrow_play_mode_count[1] == 0) {
            arrow_play_mode_count[1] = arrow_play_mode_count_orig[1];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 2:
      // Одновременно горизонтальные навстречу до половины экрана
      // Затем одновременно вертикальные до половины экрана. Далее - повторять
      arrow_complete = false;
      switch (arrow_direction) {
        case  5: arrow_complete = arrow_x[0] > stop_x[0]; break;   // Стрелка слева и справа встречаются в центре одновременно - проверять только стрелку слева
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;   // Стрелка снизу и сверху встречаются в центре одновременно - проверять только стрелку снизу
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[2]--;
          if (arrow_play_mode_count[2] == 0) {
            arrow_play_mode_count[2] = arrow_play_mode_count_orig[2];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }
        
        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 3:
      // Одновременно со всех сторон к центру
      // Завершение кадра режима - когда все стрелки собрались в центре.
      // Проверять стрелки по самой длинной стороне
      if (WIDTH >= HEIGHT)
        arrow_complete = arrow_x[0] > stop_x[0];
      else 
        arrow_complete = arrow_y[1] > stop_y[1];
        
      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[3]--;
          if (arrow_play_mode_count[3] == 0) {
            arrow_play_mode_count[3] = arrow_play_mode_count_orig[3];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }
        
        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 4:
      // Одновременно слева/справа от края до края со смещением горизонтальной оси на 1/3 высоты, далее
      // одновременно снизу/сверху от края до края со смещением вертикальной оси на 1/3 ширины
      // Завершение кадра режима - когда все стрелки собрались в центре.
      // Проверять стрелки по самой длинной стороне
      switch (arrow_direction) {
        case  5: arrow_complete = arrow_x[0] > stop_x[0]; break;   // Стрелка слева и справа движутся и достигают края одновременно - проверять только стрелку слева
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;   // Стрелка снизу и сверху движутся и достигают края одновременно - проверять только стрелку снизу
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[4]--;
          if (arrow_play_mode_count[4] == 0) {
            arrow_play_mode_count[4] = arrow_play_mode_count_orig[4];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }
        
        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 5:
      // Одновременно со всех сторон от края до края со смещением горизонтальной оси на 1/3 высоты, далее
      // Проверять стрелки по самой длинной стороне
      if (WIDTH >= HEIGHT)
        arrow_complete = arrow_x[0] > stop_x[0];
      else 
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[5]--;
          if (arrow_play_mode_count[5] == 0) {
            arrow_play_mode_count[5] = arrow_play_mode_count_orig[5];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }
        
        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;
  }

}

void arrowSetupForMode(byte mode, bool change) {
    switch (mode) {
      case 1:
        if (change) arrow_direction = 1;
        arrowSetup_mode1();    // От края матрицы к краю, по центру гориз и верт
        break;
      case 2:
        if (change) arrow_direction = 5;
        arrowSetup_mode2();    // По центру матрицы (гориз / верт) - ограничение - центр матрицы
        break;
      case 3:
        if (change) arrow_direction = 15;
        arrowSetup_mode2();    // как и в режиме 2 - по центру матрицы (гориз / верт) - ограничение - центр матрицы
        break;
      case 4:
        if (change) arrow_direction = 5;
        arrowSetup_mode4();    // От края матрицы к краю, верт / гориз
        break;
      case 5:
        if (change) arrow_direction = 15;
        arrowSetup_mode4();    // как и в режиме 4 от края матрицы к краю, на 1/3
        break;
    }
}
void arrowSetup_mode1() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = HEIGHT / 2;
    stop_x [0] = WIDTH + 7;      // скрывается за экраном на 7 пикселей
    stop_y [0] = 0;              // неприменимо 
  }    
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = WIDTH / 2;
    stop_y [1] = HEIGHT + 7;     // скрывается за экраном на 7 пикселей
    stop_x [1] = 0;              // неприменимо 
  }    
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = WIDTH - 1;
    arrow_y[2] = HEIGHT / 2;
    stop_x [2] = -7;             // скрывается за экраном на 7 пикселей
    stop_y [2] = 0;              // неприменимо 
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = HEIGHT - 1;
    arrow_x[3] = WIDTH / 2;
    stop_y [3] = -7;             // скрывается за экраном на 7 пикселей
    stop_x [3] = 0;              // неприменимо 
  }
}

void arrowSetup_mode2() {
  // Слева направо до половины экрана
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = HEIGHT / 2;
    stop_x [0] = WIDTH / 2 - 1;  // до центра экрана
    stop_y [0] = 0;              // неприменимо 
  }    
  // снизу вверх до половины экрана
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = WIDTH / 2;
    stop_y [1] = HEIGHT / 2 - 1; // до центра экрана
    stop_x [1] = 0;              // неприменимо 
  }    
  // справа налево до половины экрана
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = WIDTH - 1;
    arrow_y[2] = HEIGHT / 2;
    stop_x [2] = WIDTH / 2;      // до центра экрана
    stop_y [2] = 0;              // неприменимо 
  }
  // сверху вниз до половины экрана
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = HEIGHT - 1;
    arrow_x[3] = WIDTH / 2;
    stop_y [3] = HEIGHT / 2;     // до центра экрана
    stop_x [3] = 0;              // неприменимо 
  }
}

void arrowSetup_mode4() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = (HEIGHT / 3) * 2;
    stop_x [0] = WIDTH + 7;      // скрывается за экраном на 7 пикселей
    stop_y [0] = 0;              // неприменимо 
  }    
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = (WIDTH / 3) * 2;
    stop_y [1] = HEIGHT + 7;     // скрывается за экраном на 7 пикселей
    stop_x [1] = 0;              // неприменимо 
  }    
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = WIDTH - 1;
    arrow_y[2] = HEIGHT / 3;
    stop_x [2] = -7;             // скрывается за экраном на 7 пикселей
    stop_y [2] = 0;              // неприменимо 
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = HEIGHT - 1;
    arrow_x[3] = WIDTH / 3;
    stop_y [3] = -7;             // скрывается за экраном на 7 пикселей
    stop_x [3] = 0;              // неприменимо 
  }
}
