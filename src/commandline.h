#ifndef DL_COMMANDLINE_H
#define DL_COMMANDLINE_H

#include <QCommandLineParser>
#include <QStringList>
#include <QVector>
#include <QPair>

class CommandLine : public QCommandLineParser
{
public:
  CommandLine();

  QPair<int, char**> modifiedArgv();

  int process(int argc, char** argv);

  QString helpText() const;
  QString helpAllText() const;

  bool addOption(const QCommandLineOption& opt);
  bool addOptions(const QList<QCommandLineOption>&) = delete;

  void hideOption(const QString& name);

private:
  void pushArgv(const QString& arg);

  QString m_shortHelp, m_longHelp;
  QStringList m_args;
  QVector<QByteArray> m_argvData;
  QVector<char*> m_argvPtrs;
  QStringList m_knownOptions;
};

#endif
