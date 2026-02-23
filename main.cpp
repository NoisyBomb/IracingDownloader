#include <QCoreApplication>
#include <QDebug>

#include "iracingpathresolver.h"
#include "setupinstaller.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // ── 1. Проверяем PathResolver ──────────────────────
    IracingPathResolver resolver;

    qDebug() << "Documents path:" << resolver.documentsPath();
    qDebug() << "iRacing root:"   << resolver.iRacingRootPath();
    qDebug() << "Setups root:"    << resolver.setupsRootPath();
    qDebug() << "iRacing installed:" << resolver.isIRacingInstalled();

    // ── 2. Создаём тестовые данные ─────────────────────
    Car car("ir18", "Dallara IR-18", "dallara_ir18");

    Track track("spa", "Spa-Francorchamps", "Full Course");

    Setup setup("test_001",
                "VRS Baseline Spa",
                car,
                track,
                QUrl("https://cdn.discordapp.com/attachments/1361453755895119872/1463661891535769681/26S1-W06-GnG-LongBeach-LMP3-R.zip?ex=699d7d9c&is=699c2c1c&hm=cfbf5ffc8c582c0c18db3caae6c7cdc64d82574cdefa60727dc41791d978cec5&"),
                "vrs_spa_baseline.sto",
                "VRS");

    qDebug() << "Setup valid:"      << setup.isValid();
    qDebug() << "SubFolder name:"   << setup.subFolderName();

    // ── 3. Проверяем куда установщик собирается класть файл ──
    SetupInstaller installer(resolver);

    qDebug() << "Install path:" << installer.resolveInstallPath(setup);
    qDebug() << "Already installed?" << installer.isInstalled(setup);

    // ── 4. Пробуем установить тестовый файл ───────────
    // Создай любой файл test.sto рядом с .exe и укажи путь к нему
    QString testFile = "C:\\Users\\vladi\\Desktop\\26S1-W06-GnG-LongBeach-LMP3-R-Safe.sto";
    bool result = installer.install(setup, testFile);
    qDebug() << "Install result:" << result;

    return 0;
}
