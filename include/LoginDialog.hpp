#pragma once
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

class LoginDialog : public QDialog {
  Q_OBJECT
public:
  explicit LoginDialog(QWidget *parent = nullptr) : QDialog(parent) {
    setWindowTitle(tr("Server Login & Options"));

    // Lesen: Lokale Instanz für den Start
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "Desktop-Gallery", "upload");
    QString lastUser = settings.value("lastUser", "admin").toString();
    QString lastPath = settings.value("lastPath", "uploads/2025").toString();

    userEdit = new QLineEdit(lastUser, this);
    passEdit = new QLineEdit(this);
    passEdit->setEchoMode(QLineEdit::Password);

    pathEdit = new QLineEdit(lastPath, this);
    pathEdit->setPlaceholderText("e.g. holidays/italy");

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Username:"), userEdit);
    form->addRow(tr("Password:"), passEdit);
    form->addRow(tr("Target Folder:"), pathEdit);

    QPushButton *loginBtn = new QPushButton(tr("Login & Upload"), this);

    // KORREKTUR: 'settings' aus Capture-Liste entfernt ([this])
    connect(loginBtn, &QPushButton::clicked, this, [this]() {
      // Speichern: Wir erstellen hier eine NEUE Instanz.
      // QSettings ist optimiert, das ist kein Performance-Problem.
      QSettings s(QSettings::IniFormat, QSettings::UserScope, "Desktop-Gallery",
                  "upload");
      s.setValue("lastUser", userEdit->text());
      s.setValue("lastPath", pathEdit->text());
      accept();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(loginBtn);
  }

  QString getUser() const { return userEdit->text(); }
  QString getPass() const { return passEdit->text(); }
  QString getPath() const { return pathEdit->text(); }

private:
  QLineEdit *userEdit;
  QLineEdit *passEdit;
  QLineEdit *pathEdit;
};