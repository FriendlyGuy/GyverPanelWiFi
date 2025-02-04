![PROJECT_PHOTO](https://github.com/vvip-68/GyverPanelWiFi/blob/master/proj_img.jpg)
# Крутая WiFi панель / гирлянда на esp8266 своими руками
* [Описание проекта](#chapter-0)
* [Папки проекта](#chapter-1)
* [Схемы подключения](#chapter-2)
* [Материалы и компоненты](#chapter-3)
* [Как скачать и прошить](#chapter-4)
* [Полезная информация](#chapter-5)

<a id="chapter-0"></a>
## Описание проекта
  
Этот проект основан на проекте ["Крутая WiFi лампа на esp8266 своими руками"](https://github.com/vvip-68/GyverLampWiFi)
с расширением возможностей работы на неквадратных широких матрицах с размерами от 23x11.  
Основное назначение проекта - настенные экраны больших размеров или гирлянды на широкие балконы.  
Также подходит с некоторыми ограничениями для матриц с размерами 16x16.
Ограничение связаны с невозможностью отображения часов крупным шрифтом 5x7 для которого требуется матрица не менее 23 пикселей по ширине.
Для матриц менее 23 колонок шириной в часах может использоваться только шрифт 3x5 для которого достаточно 15 колонок.

### Железо
- Проект собран на базе микроконтроллера ESP8266 в лице платы NodeMCU или Wemos D1 mini (неважно, какую из этих плат использовать)
- Реализована поддержка микроконтроллера ESP32, имеющего больший размер оперативной памяти и быстродействие, что позволяет управлять матрицами с большим количеством светодиодов
- Матрица может быть реализована на адресной ленте или отдельных светодиодах WS8212b, спаянных в нити гирлянды проводами
- Также для компактных панелей может использоваться соединение нескольких гибких адресных матриц 16×16, состоящих из 256 диодов с плотностью 100 штук на метр, что позволяет 
  легко получить панели размерами 32х16, 48x16, 64x16 и так далее (до 128 по ширине или высоте)
- Система управляется со смартфона по Wi-Fi в локальной сети или из любой точки мира через MQTT, а также “оффлайн” с кнопки на корпусе (сенсорная кнопка на TTP223 или 
  любая физическая кнопка с нормально разомкнутыми контактами)
- В случае реализации проекта в виде большой настенной матрицы поддерживается функционал будильника-рассвет и индикация текущего времени на индикаторе TM1637, 
  что позволяет в ночное время полностью выключать саму матрицу, оставляя возможность отображения текущего времени на этом индикаторе
- Поддерживается загрузка файлов анимации формата \*.out программы Glediator с SD-карты.

### Фишки
 - Более 40 крутых эффектов с поддержкой отображения часов или текста бегущей строки поверх эффектов
 - 4 интерактивные игры: Лабиринт, Змейка, Тетрис, Арканоид (в программе-компаньоне "WiFiPlayer")
 - Отправка картинки со смартфона на матрицу (в программе-компаньоне "WiFiPlayer")
 - Рисование на матрице (в программе-компаньоне "WiFiPlayer")
 - Сохранение нарисованных изображения на SD-карте или во внутренней памяти микроконтроллера
 - Загрузка сохраненных изображений на матрицу и в программу "WiFiPlayer" для редактирования
 - Регулировка яркости эффектов относительно яркости часов или текста бегущей строки, отображаемых поверх эффектов
 - Возможность задания до 36 разных текстовых строк, задание порядка их "воспроизведения" a также параметров отображения.
   Тексты задаются из программы на смартфоне без необходимости перепрошивки контроллера.
   [Как настроить.](https://github.com/vvip-68/GyverPanelWiFi/wiki/%D0%9D%D0%B0%D1%81%D1%82%D1%80%D0%BE%D0%B9%D0%BA%D0%B0-%D0%B1%D0%B5%D0%B3%D1%83%D1%89%D0%B5%D0%B9-%D1%81%D1%82%D1%80%D0%BE%D0%BA%D0%B8)
   [Обсуждение текстов.](https://github.com/vvip-68/GyverPanelWiFi/issues/102#issue-778611501)  
 - Поддержка текста бегущей строки с отображением оставшегося до события времени, например: "До Нового года осталось 5 дней 12 часов"
   и после наступления события - вывод специального текста, например: "С Новым 2022 годом!!!"
 - Текст бегущей строки может отображаться различными цветами внутри одной строки.
 - Настройка скорости и вариаций отображения для каждого эффекта со смартфона
 - Поддержка эффектов анимации, подготовленных в программе “Jinx!”, сохраненных на SD карту
 - Работа системы как в локальной сети, так и в режиме “точки доступа”
 - Система получает точное время из Интернета
 - Управление кнопкой: смена режима, настройка яркости, вкл/выкл, отображение текущего IP адреса устройства
 - Режим будильник-рассвет: менеджер будильников на неделю в приложении
 - Отображение текущего времени на индикаторе TM1637
 - Отображение текущего времени на матрице поверх эффектов
 - Отображение текстовых сообщений на матрице поверх эффектов
 - Настройка сервера синхронизации времени из программы на смартфоне 
 - Установка текущего времени со смартфона вручную, если не удалось подключиться к серверу времени NTP
 - Два режима работы индикатора времени TM1637 - светится постоянно или выключается вместе с панелью
 - Пока время не получено с сервера NTP - на индикаторе TM1637 отображается --:-- вне зависимости от настройки
   "Выключать индикатор при выключении панели"
 - Поддержка звука будильника / звука рассвета звуковой платой MP3 DFPlayer
 - Настройки сетевого подключения (SSID и пароль, статический IP) задаются в программе и сохраняются в EEPROM
 - Если не удается подключиться к сети (неверный пароль или имя сети) - автоматически создается точка подключения
   с именем PanelAP, пароль 12341234, IP 192.168.4.1. Подключившись к точке доступа из приложения
   можно настроить параметры сети. Если после задания параметров сети WiFi соединение установлено - 
   в приложении на смартфоне виден IP адрес подключения к сети WiFi.
 - Отображение текущего IP адреса устройства на индикаторе TM1637 или на матрице в режиме бегущей строки
 - Быстрое включение популярных режимов из приложения
 - Четыре программируемых по времени режима, позволяющие, например, настроить автоматическое выключение панели в ночное время
   и автоматическое включение панели вечером в назначенное время
 - Два специальных режима времени - "Рассвет" и "Закат". Время рассвета и заката на текущий день берется с сервера погоды при получении текущих погодных условия и температуры
 - Получение текущей температуры воздуха и погоды с сервера Яндекс.Погода или OpenWeatherMap. Полученные данные могут отображаться в бегущей строке или в режиме отображения времени вместе с часами
   Код региона (города) указывается в настройках в программе на смартфоне. Настройка погоды - [тут](https://github.com/vvip-68/GyverPanelWiFi/wiki/%D0%9D%D0%B0%D1%81%D1%82%D1%80%D0%BE%D0%B9%D0%BA%D0%B0-%D0%BF%D0%BE%D0%BB%D1%83%D1%87%D0%B5%D0%BD%D0%B8%D1%8F-%D0%B8%D0%BD%D1%84%D0%BE%D1%80%D0%BC%D0%B0%D1%86%D0%B8%D0%B8-%D0%BE-%D0%BF%D0%BE%D0%B3%D0%BE%D0%B4%D0%B5)  
 - Возможность управления из любой точки планеты через подключение к MQTT-серверу из приложения на смартфоне или с использованием ассистентов умного дома (Яндекс Алиса, Google Ассистент и т.д.)  
   Настройка подключения к MQTT-серверу [здесь](https://github.com/vvip-68/GyverPanelWiFi/wiki/%D0%9D%D0%B0%D1%81%D1%82%D1%80%D0%BE%D0%B9%D0%BA%D0%B0-%D0%BF%D0%BE%D0%B4%D0%BA%D0%BB%D1%8E%D1%87%D0%B5%D0%BD%D0%B8%D1%8F-%D0%BA-MQTT-%D1%81%D0%B5%D1%80%D0%B2%D0%B5%D1%80%D1%83)  
   Описание API управления устройством [здесь](https://github.com/vvip-68/GyverPanelWiFi/wiki/API-%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D1%8F-%D1%83%D1%81%D1%82%D1%80%D0%BE%D0%B9%D1%81%D1%82%D0%B2%D0%BE%D0%BC)

#### Эффекты:
 - Заливка панели белым или другим выбранным цветом
 - Снегопад
 - Блуждающий кубик
 - Пейнтбол
 - Радуга (горизонтальная, вертикальная, диагональная, вращающаяся)
 - Огонь
 - The Matrix
 - Шарики
 - Конфетти
 - Звездопад
 - Шумовые эффекты с разными цветовыми палитрами
 - Плавная смена цвета заливки панели
 - Светлячки
 - Водоворот
 - Мерцание
 - Северное сияние
 - Циклон
 - Тени (меняющийся теневой рисунок на матрице)
 - Демо-версия игры Тетрис (автоигра без возможности управления)
 - Демо-версия игры Лабиринт (автоигра без возможности управления)
 - Демо-версия игры Змейка (автоигра без возможности управления)
 - Движущийся синус
 - Палитра (лоскутное одеяло) 
 - Имитация графического индикатора спектра, движущегося "в такт музыке". 
 - Вышиванка
 - Дождь
 - Камин
 - Водопад
 - Стрелки
 - Эволюция (симулятор жизни)
 - Погода (слайдшоу или отображение текущих погодных условия)
 - Отображение анимированных картинок
 - Фоновые узоры (нотки, сердечки, снежинки, зигзаги и т.п.
 - Анимация с SD карты

#### Игры:
 - Лабиринт
 - Змейка
 - Тетрис
 - Арканоид

### Кнопка управления режимами, последовательность переключения:
#### Будильник сработал, идет рассвет или мелодия пробуждения
- Любое нажатие кнопки отключает будильник
#### Долгое удержание кнопки 
- При включенной панели - плавное изменение яркости
- При выключенной панели - включение яркой белой панели освещения (только для сборки типа "Лампа" - DEVICE_TYPE == 0)
#### Однократное нажатие кнопки
- Включение / выключение панели. При включении возобновляется режим на котором панель была выключена.
#### Двухкратное нажатие кнопки
- Ручной переход к следующему режиму
#### Трехкратное нажатие кнопки
- Включение демо-режима с автоматической сменой режимов по циклу
#### Четырехкратное нажатие кнопки
- Включение яркой белой панели освещения из любого режима, даже если панель была "выключена" /для сборки типа "Лампа"/  
  Отображение IP адреса панели на матрице и на индикаторе TM1637, если подключение к локальной WiFi сети установлено  /для сборки типа "Панель"/  
#### Пятикратное нажатие кнопки
- На индикаторе TM1637 и на матрице отображается IP адрес панели, если подключение к локальной WiFi сети установлено /для сборки типа "Лампа"/

<a id="chapter-1"></a>
## Папки
**ВНИМАНИЕ! Если это твой первый опыт работы с Arduino, читай [инструкцию](#chapter-4)**
- **Android** - файлы с приложениями, примерами для Android и MIT App Inventor
- **firmware** - прошивки
- **libraries** - библиотеки проекта.
- **pics** - примеры картинок для загрузки / отображении на матрице
- **schemes** - схемы подключения компонентов
- **sounds** - звуковые файлы будильника для размещения на SD-карте
- **tools** - полезные утилиты, Jinx! и другие
- **wiki** - файлы раздела wiki-страниц GitHub 

<a id="chapter-2"></a>
## Схема

Варианты схем с различным набором компонент, для микроконтроллеров **ESP8266** - NodeMCU, Wemos d1 mini, **ESP32** представлены [здесь](https://github.com/vvip-68/GyverPanelWiFi/wiki/%D0%92%D0%B0%D1%80%D0%B8%D0%B0%D0%BD%D1%82%D1%8B-%D1%83%D1%81%D1%82%D1%80%D0%BE%D0%B9%D1%81%D1%82%D0%B2.-%D0%A1%D1%85%D0%B5%D0%BC%D1%8B.).

![SCHEME](https://github.com/vvip-68/GyverPanelWiFi/blob/master/schemes/schemes.png)

<a id="chapter-3"></a>
## Материалы и компоненты
### Ссылки оставлены на магазины
Полный список компонентов есть в статье https://alexgyver.ru/matrix_guide/
- NodeMCU http://ali.ski/_1FJZ, https://ali.onl/1FjU
- Wemos D1 mini http://ali.ski/FuTgbO, https://ali.onl/1FjS https://ali.onl/1Fk1
- Wemos D1 mini Pro https://ali.onl/1FjT https://ali.onl/1Fk0
- Матрица 16x16 http://ali.ski/BCKQT http://ali.ski/bRW14 http://ali.ski/X-tBrQ
- Адресная лента (для DIY матрицы) http://ali.ski/rqgqdq  http://ali.ski/4Ma9iH
- Готовая гирлянда на 20/50/100 диодов (2м/5м/10м) https://ali.onl/1Fk3 https://ali.onl/1Fk4 https://ali.onl/1Fk5
- Сенсорная кнопка http://ali.ski/aWQBAa  http://ali.ski/rsOrSB
- Резисторы http://ali.ski/UEez2
- БП 5V (брать 3A минимум) http://ali.ski/K-CThT  http://ali.ski/3UWXJ
- MP3 DFPlayer http://ali.onl/1gY1 http://ali.onl/1gY3
- SD card shield https://ali.onl/1FjV https://ali.onl/1FjW https://ali.onl/1FjZ
- Динамики http://ali.onl/1h3u https://ali.onl/1FjX
- Проводочки http://ali.ski/JQRler  http://ali.ski/_SuCF
- Дисплеи TM1637 https://ali.onl/1Fkg https://ali.onl/1Fkf
- Диоды кремниевые выпрямительные https://ali.onl/1Fmw https://ali.onl/1Fmy
- MOSFET IRF4905 https://ali.onl/1Fsl https://ali.onl/1Fsm
- MOSFET IRF3205 https://ali.onl/1Fsn https://ali.onl/1Fso
- Реле управления питанием https://ali.onl/1HBY https://ali.onl/1HBZ https://ali.onl/1HC0

## Вам скорее всего пригодится
* [Всё для пайки (паяльники и примочки)](http://alexgyver.ru/all-for-soldering/)
* [Недорогие инструменты](http://alexgyver.ru/my_instruments/)
* [Все существующие модули и сенсоры Arduino](http://alexgyver.ru/arduino_shop/)
* [Электронные компоненты](http://alexgyver.ru/electronics/)
* [Аккумуляторы и зарядные модули](http://alexgyver.ru/18650/)

<a id="chapter-4"></a>
## Как скачать и прошить
* [Первые шаги с Arduino](http://alexgyver.ru/arduino-first/) - ультра подробная статья по началу работы с Arduino, ознакомиться первым делом!
* Скачать архив с проектом
> На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**
* Установить библиотеки в  
`C:\Program Files (x86)\Arduino\libraries\` (Windows x64)  
`C:\Program Files\Arduino\libraries\` (Windows x86)
* **Подключить внешнее питание 5 Вольт**
* Подключить Arduino к компьютеру
* Запустить файл прошивки (который имеет расширение .ino)
* Настроить IDE (COM порт, модель Arduino, как в статье выше)
* Настроить что нужно по проекту
* Нажать загрузить
* Скачать и установить на смартфон GyverPanelWiFi
* Пользоваться  

**Подробная инструкция [тут](https://github.com/vvip-68/GyverPanelWiFi/wiki/%D0%9F%D0%BE%D1%88%D0%B0%D0%B3%D0%BE%D0%B2%D0%B0%D1%8F-%D0%B8%D0%BD%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%86%D0%B8%D1%8F-%D0%BF%D0%BE%D0%B4%D0%B3%D0%BE%D1%82%D0%BE%D0%B2%D0%BA%D0%B8-%D1%81%D1%80%D0%B5%D0%B4%D1%8B-%D0%B4%D0%BB%D1%8F-%D0%BF%D1%80%D0%BE%D0%B5%D0%BA%D1%82%D0%B0)**

## Важно
Если проект не собирается (ошибки компиляции) или собирается, но работает неправильно (например вся матрица светится белым и ничего не происходит) - проверьте версии библиотек. Данный проект рассчитан на работу с версиями библиотек поддержки плат ESP версии 2.5.2 и библиотеки FastLED версии 3.2.9 или более новую;

Если в качестве микроконтроллера вы используете Wemos D1 - в менеджере плат для компиляции все равно выбирайте **"NodeMCU v1.0 (ESP-12E)"**, в противном случае, если выберете плату Wemos D1 (xxxx), - будет работать нестабильно, настройки не будут сохраняться в EEPROM, параметры подключения к локальной сети будут сбрасываться каждый раз при перезагрузке, плата вместо подключения к локальной сети будет каждый раз создавать точку доступа.

<a id="chapter-5"></a>
## Полезная информация
* [Cайт Alex Gyver](http://alexgyver.ru/)
* [Канал Alex Gyver на YouTube](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)
* [YouTube канал про Arduino](https://www.youtube.com/channel/UC4axiS76D784-ofoTdo5zOA?sub_confirmation=1)
* [Видеоуроки по пайке](https://www.youtube.com/playlist?list=PLOT_HeyBraBuMIwfSYu7kCKXxQGsUKcqR)
* [Видеоуроки по Arduino](http://alexgyver.ru/arduino_lessons/)
