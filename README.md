<img width="1014" height="513" alt="image" src="https://github.com/user-attachments/assets/8151d5c3-b3a5-45be-ae95-8514ad404e7e" /># CS2 INTERNAL BASE

Внутренний C++ проект под базу для Internal Чита Counter-Strike 2. Проект собирается как x64 DLL.

## Структура

```text
main/
├─ cs2-project.sln          Решение Visual Studio
├─ cs2-project/              Исходный код DLL
│  ├─ core/                  Инициализация, хуки и меню
│  ├─ dependencies/          ImGui, MinHook, Protobuf и вспомогательные библиотеки
│  ├─ features/               Игровые функции
│  ├─ memory/                 Поиск адресов и работа с памятью
│  ├─ render/                 DirectX 11 и отрисовка интерфейса
│  └─ sdk/                    Интерфейсы и структуры CS2
├─ models/                   Дополнительные ресурсы моделей
├─ build/                    Каталог результатов сборки
└─ vcpkg_installed/          Локальные зависимости проекта
```

## Требования

- Windows 10/11 x64
- Visual Studio с workload **Desktop development with C++**
- Windows SDK
- MSVC toolset, указанный в `cs2-project/cs2-project.vcxproj`
- Запущенная установленная копия CS2 для функционального тестирования

## Сборка в Visual Studio

1. Открой `cs2-project.sln`.
2. Выбери платформу `x64`.
3. Для обычной проверки используй `Debug`; для итогового файла — `Release`.
4. Выполни `Build → Rebuild Solution`.

Результат по умолчанию:

```text
build\debug\cs2-project.dll
build\release\cs2-project.dll
```

## Сборка из Developer PowerShell

Из каталога `main\main`:

```powershell
msbuild .\cs2-project.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m:1
```

Если одновременно открыто несколько сборок, используй отдельные промежуточные каталоги:

```powershell
msbuild .\cs2-project.sln /t:Rebuild `
  /p:Configuration=Release /p:Platform=x64 /m:1 `
  /p:IntDir=x64\Release-local\
```

## Ресурсы

Ресурсы интерфейса должны находиться рядом с DLL в каталоге `assets`. Если используется конфиг или модель-чейнджер, проверь соответствующие пути в настройках проекта и не смешивай ресурсы Debug и Release.

## Диагностика сборки

- `MSB3073` обычно означает, что дочерний PowerShell/CMake-шаг завершился с ошибкой. Запусти команду из сообщения отдельно и сохрани полный вывод.
- `C1041` часто появляется при параллельной сборке. Используй `/m:1` и новый `IntDir`.
- Если DLL собрана, но интерфейс не появляется, сначала проверь инициализацию DirectX 11 и совпадение x64-разрядности.
- После обновления CS2 проверь актуальность сигнатур и offsets перед функциональным тестом.

## Ограничения

Нужно обновление под каждый патч Берите паттерны и оффсеты от сюда
https://www.cs2-sdk.com/

<img width="1014" height="513" alt="image" src="https://github.com/user-attachments/assets/f25882e7-b659-45a7-8721-8ebfc9ef087e" />

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/f59ad514-960b-4733-a48c-9e3697385e3c" />

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/1c22a2da-5c12-4791-94e5-0baa476ec7ab" />




