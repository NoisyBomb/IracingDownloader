# IracingDownloader

**Grid-and-Go Setup Manager** — десктопное приложение на Qt/C++ для поиска, скачивания и автоматической установки сетапов iRacing с платформы [Grid-and-Go](https://app.grid-and-go.com).

---

## О проекте

Приложение упрощает работу с сетапами для iRacing: вместо ручного поиска по сайту Grid-and-Go, копирования файлов и раскладывания их по папкам — всё происходит в несколько кликов.

**Ключевая особенность**: неделя в приложении начинается **в воскресенье** (на 2 дня раньше официального старта iRacing во вторник), чтобы вы могли готовить сетапы заранее.

---

##  Возможности

- **OAuth2 авторизация** через Grid-and-Go (PKCE flow) с автоматическим входом
- **Загрузка датапаков** по году и сезону через REST API
- **Фильтрация по сериям**: AdvancedMazda, Creventic, DTM, GT-Sprint, GTE-Sprint, IMSA, Majors24, NEC и другие
- **Фильтрация по неделям**: переключение между 12 гоночными неделями + off-week
- **Картинки машин и трасс** с автоматическим сопоставлением через реестры (`cars.json` / `tracks.json`)
- **Отображение лучшего круга** для каждого датапака
- ️**Разделение сетапов** на dry и wet
- **Скачивание с прогресс-баром** напрямую с S3 (подписанные URL)
- **Автоматическая установка** в `Documents/iRacing/setups/<car>/<provider>_<track>/`
- **Индикатор установленных сетапов** (зелёная галочка ✓)
- **Тёмная тема** в стиле Grid-and-Go (красный #E10600)
- **Overwrite** существующих сетапов при переустановке

---

## Использование

1. Запустите **IracingDownloader.exe**
2. Нажмите **Login** в правом верхнем углу
   <p align="center">
   <img width="882" height="114" alt="Логин" src="https://github.com/user-attachments/assets/34d6380b-4182-4ace-abb6-8d72c24d4b52" />
   </p>
3. В открывшемся окне войдите в аккаунт Grid-and-Go
   <p align="center">
   <img width="1294" height="1680" alt="Страница авторизации" src="https://github.com/user-attachments/assets/42c6cdab-0992-44a7-9087-10661b0778e7" />
   </p>
4. После авторизации автоматически загрузится список датапаков
5. Выберите нужную **серию** во вкладках
   <p align="center">
   <img width="2560" height="1590" alt="Список серий" src="https://github.com/user-attachments/assets/547ab027-d598-43db-b505-fd94605b5bad" />
   </p>
6. Используйте клавишу **Week** для фильтрации по неделе
   
    <p align="center">
       <img width="272" height="776" alt="Week тулбар для фильтрации" src="https://github.com/user-attachments/assets/07cfeedb-e08d-463e-a269-3c7d8c662445" />
    </p>
7. Нажмите **Load setups** на интересующем датапаке
   <p align="center">
   <img width="2560" height="470" alt="Клавиша для показа сетапов" src="https://github.com/user-attachments/assets/2d1e2359-82d2-41e3-8719-126c55426ae7" />
   </p>
8. Нажмите **↓** рядом с нужным сетапом
    <p align="center">
    <img width="2560" height="754" alt="Клавиши для скачиания" src="https://github.com/user-attachments/assets/22ee3824-6288-4d53-b010-6d030d2dea76" />
    </p>
9. Сетап скачается и установится в папку iRacing
    <p align="center">
    <img width="2560" height="748" alt="Статус установки" src="https://github.com/user-attachments/assets/d46ff331-6204-41dd-93c5-2461cd6bb4b5" />
    </p>
    <p align="center">
    <img width="1600" height="494" alt="Сетапы в папке соответствующей машины" src="https://github.com/user-attachments/assets/12ee3a8f-8972-49e9-a24d-70a60673c4ca" />
    </p>

---

##  Установка

### Готовый установщик

1. Перейдите в раздел [Releases](https://github.com/NoisyBomb/IracingDownloader/releases)
2. Скачайте последний `IracingDownloader_Setup.exe`
3. Запустите установщик
4. Программа появится на рабочем столе

### Системные требования

- **ОС**: Windows 10/11 (64-bit)
- **iRacing**: должен быть установлен (для автоматической установки сетапов)
- **Интернет**: для авторизации и скачивания сетапов

---

## ️ Сборка из исходников

### Требования

- **Qt 6.8.3** (MSVC 2022 64-bit или MinGW 64-bit)
- Модуль **Qt WebEngine** (для OAuth2 авторизации)
- **C++17** компилятор
- **Inno Setup 6** (опционально, для создания установщика)

### Шаги

```bash
# 1. Клонируйте репозиторий
git clone https://github.com/NoisyBomb/IracingDownloader.git
cd IracingDownloader

# 2. Откройте IracingDownloader.pro в Qt Creator

# 3. Выберите комплект (kit):
#    Desktop Qt 6.8.3 MSVC2022 64-bit

# 4. Соберите Release (Ctrl+B)

# 5. Разверните зависимости Qt
cd build\Desktop_Qt_6_8_3_MSVC2022_64bit-Release\release
windeployqt IracingDownloader.exe --release --no-compiler-runtime

# 6. Скопируйте картинки рядом с exe
xcopy ..\..\..\resources\pic pic /E /I /Y

# 7. Запустите
.\IracingDownloader.exe
