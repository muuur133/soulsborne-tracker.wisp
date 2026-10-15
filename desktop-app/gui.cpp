

#define WIN32_LEAN_AND_MEAN
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDate>
#include <QSignalBlocker>
#include <windows.h>
#include <QDialog>
#include <QGridLayout>
#include <QFrame>
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPixmap>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QString>
#include <algorithm>
#include <vector>
#include <utility>
#include <array>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Игры в порядке прохождения
const std::array<QString, 7> games = {
    "Demon's Souls",
    "Dark Souls Remastered",
    "Dark Souls II: Scholar of the First Sin",
    "Bloodborne",
    "Dark Souls III",
    "Sekiro: Shadows Die Twice",
    "Elden Ring"
};

// Имена файлов постеров
const std::array<QString, 7> posterFiles = {
    "demons_souls.jpg",
    "dark_souls_remastered.jpg",
    "dark_souls_2.jpg",
    "bloodborne.jpg",
    "dark_souls_3.jpg",
    "sekiro.jpg",
    "elden_ring.jpg"
};

// Статистика смертей
std::array<int, 7> deaths = {0, 0, 0, 0, 0, 0, 0};
int currentGame = 0;

// Путь к статистике рядом с exe
QString statsPath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath("stats.json");
}

// Загрузка статистики
void loadStats()
{
    std::ifstream file(statsPath().toStdString());

    if (!file.is_open()) {
        return;
    }

    try {
        json data;
        file >> data;

        if (data.is_object()) {
            if (data.contains("deaths") &&
                data["deaths"].is_array()) {

                const auto& savedDeaths = data["deaths"];

                for (size_t i = 0;
                     i < deaths.size() && i < savedDeaths.size();
                     ++i) {

                    if (savedDeaths[i].is_number_integer()) {
                        deaths[i] = savedDeaths[i].get<int>();

                        if (deaths[i] < 0) {
                            deaths[i] = 0;
                        }
                    }
                }
            }

            if (data.contains("currentGame") &&
                data["currentGame"].is_number_integer()) {

                currentGame = data["currentGame"].get<int>();
            }
        }

        // Проверяем номер текущей игры
        if (currentGame < 0 ||
            currentGame >= static_cast<int>(games.size())) {
            currentGame = 0;
        }
    }
    catch (...) {
        QMessageBox::warning(
            nullptr,
            "Ошибка статистики",
            "Не удалось прочитать stats.json. "
            "Сделай резервную копию файла и проверь его содержимое."
        );
    }
}

// Сохранение статистики
void saveStats()
{
    json data;
    data["deaths"] = deaths;
    data["currentGame"] = currentGame;

    std::ofstream file(statsPath().toStdString());

    if (file.is_open()) {
        file << data.dump(4);
    }
}

// Главное окно приложения
class DeathTracker : public QWidget
{
public:
    DeathTracker(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setWindowTitle("Soulsborne Death Tracker");
        setFixedSize(560, 800);

        // Основная цветовая схема
        setStyleSheet(R"(
            QWidget {
                background-color: #111214;
                color: #e8e3d9;
                font-family: "Segoe UI";
            }

            QLabel {
                background: transparent;
                color: #e8e3d9;
            }

            QLabel#titleLabel {
                color: #d7b46a;
                font-size: 20px;
                font-weight: bold;
                letter-spacing: 2px;
                padding: 5px;
            }

            QLabel#posterLabel {
                background-color: #1b1c1f;
                border: 1px solid #51452e;
                border-radius: 10px;
                color: #a7a29a;
                padding: 4px;
            }

            QLabel#gameLabel {
                color: #f0dfb5;
                font-size: 17px;
                font-weight: bold;
                padding-top: 3px;
            }

            QLabel#deathsLabel {
                color: #d9ad59;
                font-size: 27px;
                font-weight: bold;
                padding: 2px;
            }

            QLabel#hotkeysLabel {
                color: #99958d;
                font-size: 10px;
                background-color: #191a1d;
                border: 1px solid #292a2d;
                border-radius: 8px;
                padding: 9px;
            }

            QPushButton {
                background-color: #222327;
                color: #d8c69e;
                border: 1px solid #55472e;
                border-radius: 8px;
                padding: 11px 14px;
                font-size: 11px;
                font-weight: bold;
            }

            QPushButton:hover {
                background-color: #343027;
                border: 1px solid #a1844b;
                color: #fff0c8;
            }

            QPushButton:pressed {
                background-color: #171717;
                border: 1px solid #d7b46a;
            }

            QPushButton#deathButton {
                background-color: #8d302d;
                color: #fff0e5;
                border: 1px solid #bd5148;
            }

            QPushButton#deathButton:hover {
                background-color: #a83b36;
                border: 1px solid #e47769;
            }

            QPushButton#deathButton:pressed {
                background-color: #6d2523;
            }

            QPushButton#nextButton {
                background-color: #292820;
                color: #e1c47f;
                border: 1px solid #75613a;
            }

            QPushButton#nextButton:hover {
                background-color: #3b3424;
                border: 1px solid #b89a57;
            }
        )");

        // Заголовок
        titleLabel = new QLabel("SOULSBORNE DEATH TRACKER");
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setFont(QFont("Segoe UI", 17, QFont::Bold));

        // Область постера
        posterLabel = new QLabel();
        posterLabel->setObjectName("posterLabel");
        posterLabel->setAlignment(Qt::AlignCenter);
        posterLabel->setFixedSize(320, 390);
        posterLabel->setText("Загрузка постера...");

        // Название игры
        gameLabel = new QLabel();
        gameLabel->setObjectName("gameLabel");
        gameLabel->setAlignment(Qt::AlignCenter);
        gameLabel->setWordWrap(true);

        // Счётчик смертей
        deathsLabel = new QLabel();
        deathsLabel->setObjectName("deathsLabel");
        deathsLabel->setAlignment(Qt::AlignCenter);

        // Подсказки управления
        hotkeysLabel = new QLabel(
            "F8  —  засчитать смерть\n"
            "F9  —  следующая игра"
        );
        hotkeysLabel->setObjectName("hotkeysLabel");
        hotkeysLabel->setAlignment(Qt::AlignCenter);


// Кнопки управления
auto* buttonsLayout = new QVBoxLayout();
buttonsLayout->setSpacing(8);

// Первый ряд: смерть и следующая игра
auto* topRow = new QHBoxLayout();
topRow->setSpacing(8);

deathButton = new QPushButton("СМЕРТЬ +1");
deathButton->setObjectName("deathButton");
deathButton->setMinimumHeight(44);

nextButton = new QPushButton("СЛЕДУЮЩАЯ ИГРА");
nextButton->setObjectName("nextButton");
nextButton->setMinimumHeight(44);

topRow->addWidget(deathButton);
topRow->addWidget(nextButton);

// Второй ряд: статистика и дневник
auto* middleRow = new QHBoxLayout();
middleRow->setSpacing(8);

statsButton = new QPushButton("СТАТИСТИКА");
statsButton->setMinimumHeight(40);

diaryButton = new QPushButton("ДНЕВНИК ПРОХОЖДЕНИЯ");
diaryButton->setMinimumHeight(40);

middleRow->addWidget(statsButton);
middleRow->addWidget(diaryButton);

// Третий ряд: статистика прохождения
auto* diaryStatsButton =
    new QPushButton("СТАТИСТИКА ПРОХОЖДЕНИЯ");
diaryStatsButton->setMinimumHeight(40);

// Собираем три ряда
buttonsLayout->addLayout(topRow);
buttonsLayout->addLayout(middleRow);
buttonsLayout->addWidget(diaryStatsButton);


        // Главный макет
        auto* layout = new QVBoxLayout(this);
        layout->setSpacing(12);
        layout->setContentsMargins(22, 18, 22, 18);

        layout->addWidget(titleLabel);
        layout->addWidget(posterLabel, 0, Qt::AlignCenter);
        layout->addWidget(gameLabel);
        layout->addWidget(deathsLabel);
        layout->addLayout(buttonsLayout);
        layout->addWidget(hotkeysLabel);

        // Обработчики кнопок
        connect(deathButton, &QPushButton::clicked, this, [this]() {
            addDeath();
        });

        connect(nextButton, &QPushButton::clicked, this, [this]() {
            nextGame();
        });
        connect(statsButton, &QPushButton::clicked, this, [this]() {
        showStatistics();
        });
        connect(diaryButton, &QPushButton::clicked, this, [this]() {
            showDiary();
        });
        connect(diaryStatsButton, &QPushButton::clicked, this, [this]() {
            showDiaryStats();
        });
        // Глобальная горячая клавиша F8
        if (!RegisterHotKey(
                reinterpret_cast<HWND>(winId()),
                1,
                MOD_NOREPEAT,
                VK_F8)) {

            hotkeysLabel->setText(
                "F8 занята другой программой\n"
                "F9 — следующая игра"
            );
        }

        // Глобальная горячая клавиша F9
        if (!RegisterHotKey(
                reinterpret_cast<HWND>(winId()),
                2,
                MOD_NOREPEAT,
                VK_F9)) {

            hotkeysLabel->setText(
                "F8 — засчитать смерть\n"
                "Не удалось зарегистрировать F9"
            );
        }

        updateDisplay();
    }

    ~DeathTracker() override
    {
        UnregisterHotKey(reinterpret_cast<HWND>(winId()), 1);
        UnregisterHotKey(reinterpret_cast<HWND>(winId()), 2);
        saveStats();
    }

protected:
    // Обработка глобальных горячих клавиш
    bool nativeEvent(
        const QByteArray& eventType,
        void* message,
        qintptr* result) override
    {
        Q_UNUSED(eventType);

        MSG* msg = static_cast<MSG*>(message);

        if (msg->message == WM_HOTKEY) {
            if (msg->wParam == 1) {
                addDeath();
                *result = 0;
                return true;
            }

            if (msg->wParam == 2) {
                nextGame();
                *result = 0;
                return true;
            }
        }

        return QWidget::nativeEvent(eventType, message, result);
    }

private:

    QNetworkAccessManager networkManager;
    QLabel* titleLabel = nullptr;
    QLabel* posterLabel = nullptr;
    QLabel* gameLabel = nullptr;
    QLabel* deathsLabel = nullptr;
    QLabel* hotkeysLabel = nullptr;

    QPushButton* deathButton = nullptr;
    QPushButton* nextButton = nullptr;
    QPushButton* statsButton = nullptr;
    QPushButton* diaryButton = nullptr;
    
    // Отправка статистики в Supabase
    void syncToCloud()
    {
        // Читаем настройки из cloud.json рядом с exe
        QString configPath = QDir(
            QCoreApplication::applicationDirPath()
        ).filePath("cloud.json");

        QFile configFile(configPath);

        if (!configFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QJsonParseError parseError;
        QJsonDocument configDoc =
            QJsonDocument::fromJson(configFile.readAll(), &parseError);

        configFile.close();

        if (parseError.error != QJsonParseError::NoError ||
            !configDoc.isObject()) {
            return;
        }

        QJsonObject config = configDoc.object();

        QString url = config.value("url").toString();
        QString token = config.value("token").toString();

        if (url.isEmpty() || token.isEmpty()) {
            return;
        }

        // Подготавливаем семь счётчиков смертей
        QJsonArray deathsArray;

        for (int count : deaths) {
            deathsArray.append(count);
        }

        QJsonObject payload;
        payload["p_token"] = token;
        payload["p_deaths"] = deathsArray;
        payload["p_current_game"] = currentGame;

        // Адрес облачной функции
        QUrl requestUrl(
            url + "/rest/v1/rpc/update_tracker_state"
        );

        QNetworkRequest request(requestUrl);
        request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "application/json"
        );
        request.setRawHeader(
            "apikey",
            config.value("publishable_key").toString().toUtf8()
        );

        // Отправляем запрос в фоне
        networkManager.post(
            request,
            QJsonDocument(payload).toJson(QJsonDocument::Compact)
        );
    } 
    // Отправка дневника прохождения в Supabase
    void syncDiaryToCloud()
    {
        // Читаем настройки из cloud.json рядом с exe
        QString configPath = QDir(
            QCoreApplication::applicationDirPath()
        ).filePath("cloud.json");

        QFile configFile(configPath);
        if (!configFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QJsonParseError parseError;
        QJsonDocument configDoc =
            QJsonDocument::fromJson(
                configFile.readAll(),
                &parseError
            );
        configFile.close();

        if (parseError.error != QJsonParseError::NoError ||
            !configDoc.isObject()) {
            return;
        }

        QJsonObject config = configDoc.object();
        QString url = config.value("url").toString();
        QString token = config.value("token").toString();

        if (url.isEmpty() || token.isEmpty()) {
            return;
        }

        // Читаем diary.json
        QString diaryPath = QDir(
            QCoreApplication::applicationDirPath()
        ).filePath("diary.json");

        QFile diaryFile(diaryPath);
        if (!diaryFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QJsonParseError diaryError;
        QJsonDocument diaryDoc =
            QJsonDocument::fromJson(
                diaryFile.readAll(),
                &diaryError
            );
        diaryFile.close();

        if (diaryError.error != QJsonParseError::NoError ||
            !diaryDoc.isObject()) {
            return;
        }

        // Формируем запрос к защищённой функции
        QJsonObject payload;
        payload["p_token"] = token;
        payload["p_diary"] = diaryDoc.object();

        QNetworkRequest request(
            QUrl(url + "/rest/v1/rpc/update_tracker_diary")
        );

        request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "application/json"
        );

        request.setRawHeader(
            "apikey",
            config.value("publishable_key").toString().toUtf8()
        );

        networkManager.post(
            request,
            QJsonDocument(payload).toJson(QJsonDocument::Compact)
        );
    
    }
    // Обновление постера, названия и счётчика
    void updateDisplay()
    {
        gameLabel->setText(games[currentGame]);

        deathsLabel->setText(
            QString("СМЕРТЕЙ: %1").arg(deaths[currentGame])
        );

        // Путь к постеру относительно exe
        QString posterPath = QDir(
            QCoreApplication::applicationDirPath()
        ).filePath(
            "assets/posters/" + posterFiles[currentGame]
        );

        QPixmap poster(posterPath);

        if (poster.isNull()) {
            posterLabel->setPixmap(QPixmap());

            posterLabel->setText(
                "Постер не найден\n\n" +
                posterFiles[currentGame] +
                "\n\nПроверь папку:\nassets/posters"
            );
        }
        else {
            posterLabel->setText("");

            posterLabel->setPixmap(
                poster.scaled(
                    posterLabel->size() - QSize(10, 10),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                )
            );
        }

        saveStats();
        syncToCloud();
    }

    // Увеличение количества смертей
    void addDeath()
    {
        deaths[currentGame]++;
        updateDisplay();
    }

    // Переключение на следующую игру
    void nextGame()
    {
        currentGame =
            (currentGame + 1) % static_cast<int>(games.size());

        updateDisplay();
    }
    void showStatistics()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Soulsborne — Статистика");
    dialog.setFixedSize(440, 480);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #111214;
            color: #e8e3d9;
            font-family: "Segoe UI";
        }
        QLabel {
            background: transparent;
            color: #e8e3d9;
        }
        QLabel#statTitle {
            color: #d7b46a;
            font-size: 20px;
            font-weight: bold;
            padding: 8px;
        }
        QLabel#totalDeaths {
            color: #d9ad59;
            font-size: 26px;
            font-weight: bold;
            padding: 8px;
        }
        QLabel#hardestGame {
            color: #e0c68c;
            font-size: 13px;
            padding: 8px;
        }
        QFrame#statRow {
            background-color: #1b1c1f;
            border: 1px solid #34302a;
            border-radius: 6px;
        }
        QPushButton {
            background-color: #292820;
            color: #e1c47f;
            border: 1px solid #75613a;
            border-radius: 7px;
            padding: 9px;
        }
        QPushButton:hover {
            background-color: #3b3424;
        }
    )");

    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(8);
    layout->setContentsMargins(18, 16, 18, 16);

    auto* title = new QLabel("ОБЩАЯ СТАТИСТИКА");
    title->setObjectName("statTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    int total = 0;
    int hardestIndex = 0;

    for (int i = 0; i < static_cast<int>(deaths.size()); ++i) {
        total += deaths[i];

        if (deaths[i] > deaths[hardestIndex]) {
            hardestIndex = i;
        }
    }

    auto* totalLabel = new QLabel(
        QString("Всего смертей: %1").arg(total)
    );
    totalLabel->setObjectName("totalDeaths");
    totalLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(totalLabel);

    auto* hardestLabel = new QLabel(
        QString("Больше всего смертей: %1 (%2)")
            .arg(games[hardestIndex])
            .arg(deaths[hardestIndex])
    );
    hardestLabel->setObjectName("hardestGame");
    hardestLabel->setWordWrap(true);
    hardestLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(hardestLabel);

    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #51452e;");
    layout->addWidget(separator);

    for (int i = 0; i < static_cast<int>(games.size()); ++i) {
        auto* row = new QFrame();
        row->setObjectName("statRow");

        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(10, 7, 10, 7);

        auto* gameName = new QLabel(games[i]);
        gameName->setWordWrap(true);

        auto* count = new QLabel(QString::number(deaths[i]));
        count->setStyleSheet(
            "color: #d9ad59; font-weight: bold; font-size: 14px;"
        );
        count->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        rowLayout->addWidget(gameName, 1);
        rowLayout->addWidget(count);

        layout->addWidget(row);
    }

    auto* closeButton = new QPushButton("ЗАКРЫТЬ");
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked,
            &dialog, &QDialog::accept);

    dialog.exec();
}


void showDiary()
{
    QString path = QDir(
        QCoreApplication::applicationDirPath()
    ).filePath("diary.json");

    json diary = json::object();

    // Загружаем существующий дневник
    {
        std::ifstream file(path.toStdString());

        if (file.is_open()) {
            try {
                file >> diary;

                if (!diary.is_object()) {
                    diary = json::object();
                }
            }
            catch (...) {
                QMessageBox::warning(
                    this,
                    "Ошибка",
                    "Не удалось прочитать diary.json"
                );
                return;
            }
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Soulsborne — Дневник прохождения");
    dialog.setFixedSize(440, 380);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #111214;
            color: #e8e3d9;
            font-family: "Segoe UI";
        }
        QLabel {
            background: transparent;
            color: #e8e3d9;
        }
        QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {
            background-color: #222327;
            color: #e8e3d9;
            border: 1px solid #55472e;
            border-radius: 5px;
            padding: 6px;
            min-height: 22px;
        }
        QPushButton {
            background-color: #292820;
            color: #e1c47f;
            border: 1px solid #75613a;
            border-radius: 7px;
            padding: 9px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #3b3424;
        }
    )");

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(12);

    auto* title = new QLabel("ДНЕВНИК ПРОХОЖДЕНИЯ");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "color: #d7b46a; font-size: 18px; font-weight: bold;"
    );
    layout->addWidget(title);

    auto* form = new QFormLayout();
    form->setSpacing(10);

    auto* gameBox = new QComboBox();

    for (const auto& game : games) {
        gameBox->addItem(game);
    }

    auto* statusBox = new QComboBox();
    statusBox->addItems({
        "Не начата",
        "В процессе",
        "Пройдена",
        "Отложена",
        "Брошена"
    });

    auto* dateEdit = new QDateEdit();
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");
    dateEdit->setMinimumDate(QDate(2000, 1, 1));
    dateEdit->setSpecialValueText("Не указана");
    dateEdit->setDate(QDate(2000, 1, 1));

    auto* hoursBox = new QSpinBox();
    hoursBox->setRange(0, 9999);
    hoursBox->setSuffix(" ч");

    auto* minutesBox = new QSpinBox();
    minutesBox->setRange(0, 59);
    minutesBox->setSuffix(" мин");

    auto* timeWidget = new QWidget();
    auto* timeLayout = new QHBoxLayout(timeWidget);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(8);
    timeLayout->addWidget(hoursBox);
    timeLayout->addWidget(minutesBox);

    auto* ratingBox = new QDoubleSpinBox();
    ratingBox->setRange(1.0, 10.0);
    ratingBox->setSingleStep(0.5);
    ratingBox->setDecimals(1);
    ratingBox->setSpecialValueText("Не оценена");
    ratingBox->setValue(1.0);

    form->addRow("Игра:", gameBox);
    form->addRow("Статус:", statusBox);
    form->addRow("Дата прохождения:", dateEdit);
    form->addRow("Время прохождения:", timeWidget);
    form->addRow("Оценка:", ratingBox);

    layout->addLayout(form);

    auto* saveButton = new QPushButton("СОХРАНИТЬ");
    layout->addWidget(saveButton);

    // Сохраняем данные выбранной игры в памяти
    auto saveCurrentEntry = [&]() {
        const QString key = gameBox->currentText();
        json entry = json::object();

        entry["status"] = statusBox->currentText().toStdString();

        if (dateEdit->date() != QDate(2000, 1, 1)) {
            entry["date"] = dateEdit->date()
                .toString("yyyy-MM-dd").toStdString();
        } else {
            entry["date"] = nullptr;
        }

        entry["hours"] = hoursBox->value();
        entry["minutes"] = minutesBox->value();

        if (ratingBox->value() > 1.0) {
            entry["rating"] = ratingBox->value();
        } else {
            entry["rating"] = nullptr;
        }

        diary[key.toStdString()] = entry;
    };

    // Загружаем данные выбранной игры
    auto loadCurrentEntry = [&]() {
        const QString key = gameBox->currentText();

        const QSignalBlocker blockDate(dateEdit);
        const QSignalBlocker blockHours(hoursBox);
        const QSignalBlocker blockMinutes(minutesBox);
        const QSignalBlocker blockRating(ratingBox);
        const QSignalBlocker blockStatus(statusBox);

        dateEdit->setDate(QDate(2000, 1, 1));
        hoursBox->setValue(0);
        minutesBox->setValue(0);
        ratingBox->setValue(1.0);
        statusBox->setCurrentIndex(0);

        const std::string name = key.toStdString();

        if (!diary.contains(name) || !diary[name].is_object()) {
            return;
        }

        const json& entry = diary[name];

        if (entry.contains("status") && entry["status"].is_string()) {
            QString savedStatus = QString::fromStdString(
                entry["status"].get<std::string>()
            );

            int index = statusBox->findText(savedStatus);

            if (index >= 0) {
                statusBox->setCurrentIndex(index);
            }
        }

        if (entry.contains("date") && entry["date"].is_string()) {
            QDate date = QDate::fromString(
                QString::fromStdString(
                    entry["date"].get<std::string>()
                ),
                "yyyy-MM-dd"
            );

            if (date.isValid()) {
                dateEdit->setDate(date);
            }
        }

        if (entry.contains("hours") &&
            entry["hours"].is_number_integer()) {
            hoursBox->setValue(entry["hours"].get<int>());
        }

        if (entry.contains("minutes") &&
            entry["minutes"].is_number_integer()) {
            minutesBox->setValue(entry["minutes"].get<int>());
        }

        if (entry.contains("rating") &&
            entry["rating"].is_number()) {
            ratingBox->setValue(entry["rating"].get<double>());
        }
    };

    // При смене игры сохраняем предыдущие поля в памяти
    connect(
        gameBox,
        &QComboBox::currentIndexChanged,
        &dialog,
        [&](int) {
            saveCurrentEntry();
            loadCurrentEntry();
        }
    );

    // Сохраняем дневник в файл
    connect(saveButton, &QPushButton::clicked, &dialog, [&]() {
        saveCurrentEntry();

        std::ofstream file(path.toStdString());

        if (!file.is_open()) {
            QMessageBox::warning(
                &dialog,
                "Ошибка",
                "Не удалось сохранить diary.json"
            );
            return;
        }

        
file << diary.dump(4);
file.close();

if (!file) {
    QMessageBox::warning(
        &dialog,
        "Ошибка",
        "Не удалось полностью записать дневник"
    );
    return;
}

// Отправляем сохранённый дневник в Supabase
syncDiaryToCloud();

QMessageBox::information(
    &dialog,
    "Готово",
    "Дневник прохождения сохранён!"
);
    });

    // Открываем текущую игру
    gameBox->setCurrentIndex(currentGame);
    loadCurrentEntry();

    dialog.exec();
}

void showDiaryStats()
{
    QString path = QDir(
        QCoreApplication::applicationDirPath()
    ).filePath("diary.json");

    json diary = json::object();

    std::ifstream file(path.toStdString());

    if (file.is_open()) {
        try {
            file >> diary;

            if (!diary.is_object()) {
                diary = json::object();
            }
        }
        catch (...) {
            QMessageBox::warning(
                this,
                "Ошибка",
                "Не удалось прочитать diary.json"
            );
            return;
        }
    }

    int completed = 0;
    int totalMinutes = 0;
    double ratingSum = 0.0;
    int ratedGames = 0;

    std::vector<std::pair<std::string, double>> favorites;

    for (const auto& item : diary.items()) {
        const json& entry = item.value();

        if (!entry.is_object()) {
            continue;
        }

        if (entry.contains("status") &&
            entry["status"].is_string() &&
            entry["status"] == "Пройдена") {
            completed++;
        }

        int hours = 0;
        int minutes = 0;

        if (entry.contains("hours") &&
            entry["hours"].is_number_integer()) {
            hours = entry["hours"].get<int>();
        }

        if (entry.contains("minutes") &&
            entry["minutes"].is_number_integer()) {
            minutes = entry["minutes"].get<int>();
        }

        totalMinutes += hours * 60 + minutes;

        if (entry.contains("rating") &&
            entry["rating"].is_number()) {
            double rating = entry["rating"].get<double>();

            if (rating > 1.0 && rating <= 10.0) {
                ratingSum += rating;
                ratedGames++;

                favorites.push_back({item.key(), rating});
            }
        }
    }

    std::sort(
        favorites.begin(),
        favorites.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        }
    );

    QDialog dialog(this);
    dialog.setWindowTitle("Soulsborne — Статистика прохождения");
    dialog.setMinimumSize(440, 500);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #111214;
            color: #e8e3d9;
            font-family: "Segoe UI";
        }
        QLabel {
            background: transparent;
            color: #e8e3d9;
        }
        QPushButton {
            background-color: #292820;
            color: #e1c47f;
            border: 1px solid #75613a;
            border-radius: 7px;
            padding: 9px;
        }
    )");

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(22, 20, 22, 20);
    layout->setSpacing(14);

    auto* title = new QLabel("СТАТИСТИКА ПРОХОЖДЕНИЯ");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "color: #d7b46a; font-size: 18px; font-weight: bold;"
    );
    layout->addWidget(title);

    int totalGames = static_cast<int>(games.size());

    auto* progress = new QLabel(
        QString("Пройдено игр: %1 из %2")
            .arg(completed)
            .arg(totalGames)
    );
    progress->setStyleSheet("font-size: 15px; font-weight: bold;");
    layout->addWidget(progress);

    auto* time = new QLabel(
        QString("Общее время: %1 ч %2 мин")
            .arg(totalMinutes / 60)
            .arg(totalMinutes % 60)
    );
    layout->addWidget(time);

    QString averageText = "Средняя оценка: —";

    if (ratedGames > 0) {
        averageText = QString("Средняя оценка: %1 / 10")
            .arg(ratingSum / ratedGames, 0, 'f', 1);
    }

    layout->addWidget(new QLabel(averageText));

    auto* favoritesTitle = new QLabel("ЛЮБИМЫЕ ИГРЫ");
    favoritesTitle->setStyleSheet(
        "color: #d7b46a; font-size: 15px; font-weight: bold;"
    );
    layout->addWidget(favoritesTitle);

    QString favoritesText;

    if (favorites.empty()) {
        favoritesText = "Пока нет выставленных оценок.";
    } else {
        int limit = std::min(5, static_cast<int>(favorites.size()));

        for (int i = 0; i < limit; i++) {
            favoritesText += QString("%1. %2 — %3/10\n")
                .arg(i + 1)
                .arg(QString::fromStdString(favorites[i].first))
                .arg(favorites[i].second, 0, 'f', 1);
        }
    }

    auto* favoritesLabel = new QLabel(favoritesText);
    favoritesLabel->setWordWrap(true);
    layout->addWidget(favoritesLabel);

    layout->addStretch();

    auto* closeButton = new QPushButton("ЗАКРЫТЬ");
    layout->addWidget(closeButton);

    connect(
        closeButton,
        &QPushButton::clicked,
        &dialog,
        &QDialog::accept
    );

    dialog.exec();
}
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    loadStats();

    DeathTracker window;
    window.show();

    return app.exec();
}