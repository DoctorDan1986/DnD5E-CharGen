#include "dnd/character.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QWidget>
#include <array>
#include <filesystem>
#include <string>

namespace {
std::filesystem::path asPath(const QString& str) {
    #ifdef _WIN32
    return std::filesystem::path(str.toStdWString());
#else
    return std::filesystem::path(str.toUtf8().constData());
#endif
}

class CharacterWindow : public QWidget {
public:
    explicit CharacterWindow(dnd::ContentRegistry registry) : registry_(std::move(registry)) {
        setWindowTitle("5E Character Generator — 2014 (Foundation)");
        resize(650, 660);
        auto* layout = new QVBoxLayout(this);
        auto* intro = new QLabel("2014 character builder foundation — core and homebrew content autoloaded");
        intro->setWordWrap(true);
        layout->addWidget(intro);

        auto* selectionBox = new QGroupBox("Race and subrace");
        auto* selection = new QFormLayout(selectionBox);
        race_ = new QComboBox;
        subrace_ = new QComboBox;
        selection->addRow("Race", race_);
        selection->addRow("Subrace", subrace_);
        layout->addWidget(selectionBox);

        auto* abilitiesBox = new QGroupBox("Base abilities (before racial modifiers)");
        auto* grid = new QGridLayout(abilitiesBox);
        const std::array<int, 6> defaults = {15, 14, 13, 12, 10, 8};
        for (int i = 0; i < 6; ++i) {
            grid->addWidget(new QLabel(dnd::abilityNames[i]), i / 3, (i % 3) * 2);
            auto* spin = new QSpinBox;
            spin->setRange(3, 20);
            spin->setValue(defaults[i]);
            grid->addWidget(spin, i / 3, (i % 3) * 2 + 1);
            abilities_[i] = spin;
            connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, [this] { updatePreview(); });
        }
        layout->addWidget(abilitiesBox);

        auto* caption = new QLabel("Calculated preview (not yet a complete character sheet)");
        layout->addWidget(caption);
        preview_ = new QPlainTextEdit;
        preview_->setReadOnly(true);
        layout->addWidget(preview_, 1);

        const auto races = registry_.ofKind("race", "5e-2014");
        for (const auto* r : races)
            race_->addItem(QString::fromStdString(r->name), QString::fromStdString(r->id));
        connect(race_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this] { updateSubraces(); });
        connect(subrace_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this] { updatePreview(); });
        updateSubraces();

        if (!registry_.diagnostics().empty()) {
            QString detail = QString::number(registry_.diagnostics().size()) + " content diagnostic(s):\n";
            for (const auto& diagnostic : registry_.diagnostics())
                detail += QString::fromStdString(diagnostic.file + ": " + diagnostic.message + "\n");
            auto* warnings = new QPlainTextEdit(detail);
            warnings->setReadOnly(true);
            warnings->setMaximumHeight(105);
            layout->addWidget(warnings);
        }
    }

private:
    void updateSubraces() {
        subrace_->blockSignals(true);
        subrace_->clear();
        subrace_->addItem("(None)", "");
        const auto selected = race_->currentData().toString().toStdString();
        for (const auto* s : registry_.ofKind("subrace", "5e-2014")) {
            if (std::string(s->data.at("parent").as_string()) == selected)
                subrace_->addItem(QString::fromStdString(s->name), QString::fromStdString(s->id));
        }
        if (subrace_->count() > 1) subrace_->setCurrentIndex(1);
        subrace_->blockSignals(false);
        updatePreview();
    }

    void updatePreview() {
        try {
            dnd::CharacterChoices choices;
            choices.raceId = race_->currentData().toString().toStdString();
            choices.subraceId = subrace_->currentData().toString().toStdString();
            for (int i = 0; i < 6; ++i) choices.baseAbilities[i] = abilities_[i]->value();
            const auto result = dnd::resolveCharacter(registry_, choices);
            QString output = "2014 5e • Level 1\n\n";
            for (int i = 0; i < 6; ++i) {
                output += QString("%1   %2   (%3%4)\n")
                    .arg(dnd::abilityNames[i])
                    .arg(result.abilities[i])
                    .arg(result.modifiers[i] >= 0 ? "+" : "")
                    .arg(result.modifiers[i]);
            }
            output += "\nSize: " + QString::fromStdString(result.size);
            output += "\nSpeed: " + QString::number(result.speed) + " ft.";
            output += "\nExtra HP from traits: +" + QString::number(result.bonusHitPoints);
            output += "\n\nTraits:\n";
            for (const auto& id : result.appliedTraits) {
                const auto* trait = registry_.find(id);
                output += "• " + QString::fromStdString(trait ? trait->name : id) + "\n";
            }
            if (!result.appliedFeats.empty()) {
                output += "\nFeats:\n";
                for (const auto& id : result.appliedFeats) {
                    const auto* feat = registry_.find(id);
                    output += "• " + QString::fromStdString(feat ? feat->name : id) + "\n";
                }
            }
            if (!result.senses.empty()) {
                output += "\nSenses:\n";
                for (const auto& sense : result.senses)
                    output += "• " + QString::fromStdString(sense) + "\n";
            }
            preview_->setPlainText(output);
        } catch (const std::exception& e) {
            preview_->setPlainText(QString::fromStdString(e.what()));
        }
    }

    dnd::ContentRegistry registry_;
    QComboBox* race_ = nullptr;
    QComboBox* subrace_ = nullptr;
    std::array<QSpinBox*, 6> abilities_{};
    QPlainTextEdit* preview_ = nullptr;
};
}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    const QDir exeDir(QCoreApplication::applicationDirPath());
    const QString userContent = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Content";
    QDir().mkpath(userContent);
    std::vector<std::filesystem::path> roots = {
        asPath(exeDir.filePath("Content")),
        asPath(exeDir.filePath("../Content")),
        asPath(exeDir.filePath("../../../Content")), // macOS .app beside Content/
        asPath(userContent)
    };
#ifdef DND_SOURCE_DIR
    roots.push_back(std::filesystem::path(DND_SOURCE_DIR) / "Content");
#endif
    dnd::ContentRegistry registry;
    registry.loadMany(roots);
    CharacterWindow window(std::move(registry));
    window.show();
    return app.exec();
}
