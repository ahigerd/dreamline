#ifndef DL_DLAPPLICATION_H
#define DL_DLAPPLICATION_H

#include <QApplication>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QCommandLineOption>
#include <QMultiMap>
#include <map>
#include <memory>

class DLApplication {
Q_DECLARE_TR_FUNCTIONS(DLApplication)
public:
  DLApplication(int argc, char** argv);

  void addOption(const QCommandLineOption& opt);

  bool parseCommandLine();
  bool processCommandLine(int* exitCode);
  int exec();

  QString value(const QString& name, const QString& defaultValue = QString()) const;
  QStringList values(const QString& name) const;
  bool isSet(const QString& name) const;
  int count(const QString& name) const;
  QStringList positionalArguments() const;

  QApplication* qtApp() const;

private:
  const QCommandLineOption* findOption(const QString& name, bool* isQtOpt) const;
  void addArgv(const QString& arg);

  void showHelp(bool addQtOpts) const;
  void showHelp(const QCommandLineOption& opt, int columnWidth, int lineWidth) const;
  void showVersion() const;
  void showUnknownError(const QString& optName) const;
  void showUnexpectedValueError(const QString& optName) const;
  void showMissingValueError(const QString& optName) const;

  // Option definitions
  QList<QCommandLineOption> m_dlOpts, m_qtOpts;
  std::map<QString, QCommandLineOption> m_dlOptNames, m_qtOptNames;
  QMap<QString, QString> m_canonicalNames;

  // Provided command line arguments
  QStringList m_args;
  QMultiMap<QString, QString> m_values;
  QMap<QString, int> m_optionCount;
  QStringList m_positional;

  // Data for QApplication
  int m_argc;
  QVector<QByteArray> m_argvData;
  QVector<char*> m_argvPtrs;
  std::unique_ptr<QApplication> m_qtApp;
};

#endif
