#pragma once
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

/**
 * @class LoginDialog
 * @brief A dialog for user authentication and upload configuration.
 *
 * Allows the user to enter their credentials and target path for uploading
 * images to the remote web gallery server.
 */
class LoginDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * @brief Constructor.
   * @param parent The parent widget.
   */
  explicit LoginDialog(QWidget *parent = nullptr) : QDialog(parent) {
    setWindowTitle(tr("Server Login & Options"));

    // Read: Local instance for startup
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

    // CORRECTION: Removed 'settings' from capture list ([this])
    connect(loginBtn, &QPushButton::clicked, this, [this]() {
      // Save: We create a NEW instance here.
      // QSettings is optimized, so this is not a performance issue.
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

  /**
   * @brief Get the entered username.
   * @return The username string.
   */
  QString getUser() const { return userEdit->text(); }

  /**
   * @brief Get the entered password.
   * @return The password string.
   */
  QString getPass() const { return passEdit->text(); }

  /**
   * @brief Get the target upload path.
   * @return The path string.
   */
  QString getPath() const { return pathEdit->text(); }

private:
  QLineEdit *userEdit;
  QLineEdit *passEdit;
  QLineEdit *pathEdit;
};