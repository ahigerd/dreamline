#include "commandline.h"
#include <QApplication>
#include <private/qapplication_p.h>
#include <QFile>
#include <iostream>

/* XXX: Due to limitations in QCommandLineParser and QCoreApplication, this
 * functionality is hacky:
 *
 * - QCommandLineParser::helpText() does not include Qt options, even if they
 *   are rendered by QCommandLineParser::process().
 *     - This class must therefore collect Qt options and add them explicitly.
 * - QCommandLineParser uses private functions from QCoreApplication and
 *   QApplication to collect information about Qt options.
 *     - This class must therefore include private headers.
 * - QCoreApplication consumes anything that looks like a Qt option, even if it
 *   would have been recognized by the QCommandLineParser configuration.
 *     - This class must therefore parse the raw inputs before Qt sees them.
 * - Despite the above, QCommandLineParser cannot generate help text without
 *   having first constructed a QCoreApplication, even if it does not need to
 *   support Qt options.
 *     - This class must therefore construct a dummy QApplication.
 * - QCommandLineParser does not support / as a sigil, despite this being the
 *   standard command-line option sigil on Windows.
 *     - This class must therefore preprocess the inputs and postprocess the
 *       generated help text.
 * - QCommandLineParser is basically write-only, providing no API to inspect
 *   what options have already been populated.
 *     - This class must therefore shadow some methods in order to collect
 *       information that would otherwise be unavailable.
 */

static QString readArg(char* rawArg)
{
  QString arg = QString::fromLocal8Bit(rawArg);
#ifdef Q_OS_WIN
  if (arg.startsWith("/")) {
    if (arg == "/?") {
      arg = "--help";
    } else if (arg.length() == 2) {
      arg = "-" + arg.mid(1);
    } else {
      arg = "--" + arg.mid(1);
    }
  }
#endif
  return arg;
}

static QString updateHelpText(const QString& source)
{
#ifdef Q_OS_WIN
  QStringList result;
  QStringList lines = source.split('\n');
  int columnWidth = -1;
  for (QString line : lines) {
    if (!line.startsWith("  -")) {
      result << line;
      continue;
    }
    if (columnWidth < 0) {
      for (int i = 5; i < line.length() - 2; i++) {
        if (line[i] == ' ' && line[i + 1] == ' ' && line[i + 2] != ' ') {
          columnWidth = i + 2;
          break;
        }
      }
    }
    QStringList names = line.left(columnWidth).trimmed().split(", ");
    QString rightColumn = line.mid(columnWidth);
    for (QString& name : names) {
      if (name.startsWith("--")) {
        name = "/" + name.mid(2);
      } else {
        name = "/" + name.mid(1);
      }
    }
    result << QStringLiteral("  %1  %2").arg(names.join(", "), 4 - columnWidth).arg(rightColumn);
  }
  return result.join("\n");
#else
  return source;
#endif
}

CommandLine::CommandLine()
: QCommandLineParser()
{
  addHelpOption();
  addVersionOption();
}

bool CommandLine::addOption(const QCommandLineOption& opt)
{
  bool ok = QCommandLineParser::addOption(opt);
  if (ok) {
    m_knownOptions << opt.names();
  }
  return ok;
}

void CommandLine::hideOption(const QString& name)
{
  m_knownOptions << name;
}

QPair<int, char**> CommandLine::modifiedArgv()
{
  return QPair<int, char**>(m_argvPtrs.size(), m_argvPtrs.data());
}

int CommandLine::process(int argc, char** argv)
{
  QList<QCommandLineOption> qtOpts;

  bool foundDoubleDash = false;
  m_args << QString::fromLocal8Bit(argv[0]);
  pushArgv(m_args[0]);

  for (int i = 1; i < argc; i++) {
    // Convert /option to --option unless it follows "--"
    QString arg = !foundDoubleDash ? readArg(argv[i]) : QString::fromLocal8Bit(argv[i]);
    m_args << arg;
    if (arg == "--") {
      foundDoubleDash = true;
    }
  }

  // Construct a dummy QApplication with minimal arguments in
  // order to collect the help text including the Qt options.
  {
    int dummyArgc = 1;
    QApplication app(dummyArgc, (char*[]){ argv[0] });
    m_shortHelp = QCommandLineParser::helpText();
    QList<QCommandLineOption> rawQtOpts;
    QApplicationPrivate::instance()->addQtOptions(&rawQtOpts);
    for (const QCommandLineOption& qtOpt : rawQtOpts) {
      bool suppress = false;
      for (const QString& name : qtOpt.names()) {
        if (m_knownOptions.contains(name)) {
          suppress = true;
          break;
        }
      }
      if (!suppress) {
        qtOpts << qtOpt;
        QCommandLineParser::addOption(qtOpt);
      }
    }
    m_longHelp = QCommandLineParser::helpText();
  }

  bool ok = parse(m_args);

  if (isSet("help-all")) {
    std::cout << qPrintable(helpAllText()) << std::endl;
    return -1;
  }

  if (isSet("help")) {
    std::cout << qPrintable(helpText()) << std::endl;
    return -1;
  }

  if (isSet("version")) {
    showVersion();
    return -1;
  }

  if (!ok) {
    std::cerr << qPrintable(m_args[0]) << ": " << qPrintable(errorText()) << std::endl << std::endl;
    std::cerr << qPrintable(helpText()) << std::endl;
    return 1;
  }

  // If it's an option that Qt is going to process itself,
  // be sure to include it in
  QStringList activeOptions = optionNames();
  for (const QCommandLineOption& opt : qtOpts) {
    for (const QString& name : opt.names()) {
      if (!activeOptions.contains(name)) {
        continue;
      }
      QString dashName = "--" + name;
      if (opt.valueName().isEmpty()) {
        pushArgv(dashName);
      } else {
        for (const QString& value : values(opt)) {
          pushArgv(dashName);
          pushArgv(value);
        }
      }
      break;
    }
  }

  return 0;
}

QString CommandLine::helpText() const
{
  return updateHelpText(m_shortHelp);
}

QString CommandLine::helpAllText() const
{
  return updateHelpText(m_longHelp);
}

void CommandLine::pushArgv(const QString& arg)
{
  m_argvData << arg.toLocal8Bit();
  m_argvPtrs << m_argvData.last().data();
}
