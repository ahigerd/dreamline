#include "dlapplication.h"
#include <QApplication>
#include <private/qapplication_p.h>
#include <iostream>

/*
 * DLApplication mostly exists to work around limitations in Qt's built-in
 * command-line parsing. There are two specific issues:
 * - QApplication mutates argv to remove the options that it consumed.
 *   However, its parser is naive and can't tell if something that looks like
 *   a Qt option is actually a parameter to a different option.
 * - QCommandLineParser doesn't integrate very nicely with Windows. It only
 *   supports -option and --option, but not /option. It also doesn't provide
 *   any control over its help formatting.
 *
 * As an additional wart of Qt's built-in command-line handling, there is no
 * public API to collect the flags that QApplication recognizes. This means
 * that the only way to work around the aforementioned issues is to use APIs
 * from private Qt headers.
 */

// These Qt options are useless for Dreamline.
// They will be hidden from the help text.
static QStringList hiddenQtOptions{
  "qmljsdebugger",
  "qwindowtitle",
  "title",
};

QPair<QString, QString> parseOption(const QString& arg)
{
  QString content, value;
#ifdef Q_OS_WIN
  if (arg.startsWith("/")) {
    content = arg.mid(1);
  } else
#endif
  if (arg.startsWith("--")) {
    content = arg.mid(2);
  } else if (arg.startsWith("-")) {
    content = arg.mid(1);
  }

  if (content.isEmpty()) {
    return qMakePair(QString(), arg);
  }

  int valuePos = content.indexOf('=');
  // Option names must be at least one character long. If the = is in position
  // 0, that means the user typed something like --= or -= or /=, which is not
  // a valid option. Treat it as a positional argument.
  if (valuePos == 0) {
    return qMakePair(QString(), arg);
  }

  // If there is no equals sign, don't return a value.
  if (valuePos < 0) {
    return qMakePair(content, QString());
  }

  // We intentionally preserve the = so that we can distinguish
  // between an empty value and no value.
  return qMakePair(content.left(valuePos), content.mid(valuePos));
}

DLApplication::DLApplication(int argc, char** argv)
{
  for (int i = 0; i < argc; i++)
  {
    m_args << QString::fromLocal8Bit(argv[i]);
  }

  // This mutates argc and argv but that's okay: we've already copied them
  QApplication tempApp(argc, argv);
  // The only way to get the Qt options is through private headers.
  QApplicationPrivate::instance()->addQtOptions(&m_qtOpts);
  for (QCommandLineOption& opt : m_qtOpts) {
    for (const QString& name : opt.names()) {
      if (hiddenQtOptions.contains(name)) {
        opt.setFlags(opt.flags() | QCommandLineOption::HiddenFromHelp);
        break;
      }
    }
    for (const QString& name : opt.names()) {
      // This uses std::map instead of QMap because QMap requires
      // the data type to be default-constructible.
      m_qtOptNames.emplace(name, opt);
    }
  }

  addOption({
    QStringList{
#ifdef Q_OS_WIN
      "?",
#endif
      "h", "help" }, tr("Displays help on command-line options.")
  });
  addOption({
    QStringList{ "help-all" }, tr("Displays help including Qt-specific options.")
  });
  addOption({
    QStringList{ "v", "version" }, tr("Displays version information.")
  });
}

void DLApplication::addOption(const QCommandLineOption& opt)
{
  m_dlOpts << opt;

  QString canonicalName;
  for (const QString& name : opt.names()) {
    if (name.length() > canonicalName.length()) {
      canonicalName = name;
    }
  }

  for (const QString& name : opt.names()) {
    m_dlOptNames.emplace(name, opt);
    if (!m_canonicalNames.contains(name)) {
      m_canonicalNames[name] = canonicalName;
    }
  }
}

static QString canonicalOptionName(const QCommandLineOption* opt)
{
  QString canonicalName;
  for (const QString& name : opt->names()) {
    if (name.length() > canonicalName.length()) {
      canonicalName = name;
    }
  }
  return canonicalName;
}

const QCommandLineOption* DLApplication::findOption(const QString& name, bool* isQtOpt) const
{
  auto iter = m_dlOptNames.find(name);
  if (iter != m_dlOptNames.end()) {
    *isQtOpt = false;
    return &iter->second;
  } else {
    iter = m_qtOptNames.find(name);
    if (iter != m_qtOptNames.end()) {
      *isQtOpt = true;
      return &iter->second;
    }
  }
  return nullptr;
}

void DLApplication::addArgv(const QString& arg)
{
  m_argc++;
  m_argvData << arg.toLocal8Bit();
  m_argvPtrs << m_argvData.last().data();
}

bool DLApplication::parseCommandLine()
{
  // If parseCommandLine() is called more than once, be sure
  // to clear the previous results.
  m_argc = 0;
  m_argvData.clear();
  m_argvPtrs.clear();
  m_positional.clear();
  m_values.clear();
  m_optionCount.clear();

  // Always pass in the application name.
  if (m_args.isEmpty()) {
    qFatal("Invalid command line provided to DLApplication");
  }
  addArgv(m_args[0]);

  bool hasError = false;
  int numArgs = m_args.length();
  bool foundDoubleDash = false;
  for (int i = 1; i < numArgs; i++) {
    QString arg = m_args[i];

    // -- indicates that all following arguments are positional
    // and should be handled as-is, not parsed.
    if (foundDoubleDash) {
      m_positional << arg;
      continue;
    } else if (arg == "--") {
      foundDoubleDash = true;
      continue;
    }

    auto parsed = parseOption(arg);
    QString optName = parsed.first;
    QString optValue = parsed.second;

    // parseOption said this is a positional parameter.
    if (optName.isEmpty()) {
      m_positional << optValue;
      continue;
    }

    bool isQtOpt = false;
    const QCommandLineOption* opt = findOption(optName, &isQtOpt);
    if (!opt) {
      showUnknownError(optName);
      hasError = true;
      continue;
    }

    if (isQtOpt) {
      addArgv("--" + optName);
    }

    bool valueRequired = !opt->valueName().isEmpty();

    // If the option does not require a value, but one was provided, error.
    if (!valueRequired && !optValue.isEmpty()) {
      showUnexpectedValueError(optName);
      hasError = true;
      continue;
    }

    if (valueRequired) {
      // If the option requires a value and the = syntax wasn't used, consume
      // the next argument in the input. Throw an error if one isn't present.
      if (optValue.isEmpty()) {
        if (i == numArgs - 1) {
          showMissingValueError(optName);
          hasError = true;
          continue;
        }
        optValue = m_args[++i];
      } else {
        optValue = optValue.mid(1);
      }

      // Qt options just go in the modified argv.
      // Non-Qt options go where we can look up the values.
      if (isQtOpt) {
        addArgv(optValue);
      } else {
        m_values.insert(opt->valueName(), optValue);
      }
    }

    // Look up the "canonical" name of the option and use that to store that
    // the option was provided.
    QString canonicalName = canonicalOptionName(opt);
    m_optionCount[canonicalName] = m_optionCount.value(canonicalName, 0) + 1;
  }

  return !hasError;
}

bool DLApplication::processCommandLine(int* exitCode)
{
  // If the command line hasn't been parsed already, parse it.
  if (m_argvData.isEmpty() && !parseCommandLine()) {
    *exitCode = 1;
    return true;
  }

  // Show help if requested, and exit afterward.
  if (isSet("help") || isSet("help-all")) {
    showHelp(isSet("help-all"));
    *exitCode = 0;
    return true;
  }

  // Show version information if requested, and exit afterward.
  if (isSet("version")) {
    showVersion();
    *exitCode = 0;
    return true;
  }

  // If we didn't already exit, build a QApplication.
  m_qtApp.reset(new QApplication(m_argc, m_argvPtrs.data()));
  return false;
}

QString DLApplication::value(const QString& name, const QString& defaultValue) const
{
  return m_values.value(name, defaultValue);
}

QStringList DLApplication::values(const QString& name) const
{
  return m_values.values(name);
}

bool DLApplication::isSet(const QString& name) const
{
  return count(name) > 0;
}

int DLApplication::count(const QString& name) const
{
  QString canonicalName = m_canonicalNames.value(name);
  if (canonicalName.isEmpty()) {
    qWarning("Unknown option name: %s", qPrintable(name));
    return 0;
  }
  return m_optionCount.value(canonicalName);
}

QStringList DLApplication::positionalArguments() const
{
  return m_positional;
}

QApplication* DLApplication::qtApp() const
{
  return m_qtApp.get();
}

int DLApplication::exec()
{
  return m_qtApp->exec();
}

void DLApplication::showHelp(bool addQtOpts) const
{
  int columnWidth = 0;
  for (const QCommandLineOption& opt : m_dlOpts + m_qtOpts) {
    int w = 2;
    for (const QString& name : opt.names()) {
#ifdef Q_OS_WIN
      w += 3 + name.length();
#else
      if (name.length() == 1) {
        w += 4;
      } else {
        w += 4 + name.length();
      }
#endif
    }
    if (w > columnWidth) {
      columnWidth = w;
    }
  }

  int lineWidth = qEnvironmentVariableIntValue("COLUMNS");
  if (!lineWidth) {
    lineWidth = 80;
  }

  std::cout << qPrintable(tr("Usage:")) << " "
    << qPrintable(m_args[0]) << " "
    << qPrintable(tr("[options]")) << " "
    << qPrintable(QStringLiteral("[%1...]").arg(tr("paths"))) << std::endl;
  std::cout << std::endl;
  std::cout << qPrintable(tr("Dreamline Options:")) << std::endl;
  for (const QCommandLineOption& opt : m_dlOpts) {
    showHelp(opt, columnWidth, lineWidth);
  }

  if (addQtOpts) {
    std::cout << std::endl;
    std::cout << qPrintable(tr("Qt Options:")) << std::endl;
    for (const QCommandLineOption& opt : m_qtOpts) {
      showHelp(opt, columnWidth, lineWidth);
    }
  }

  std::cout << std::endl;
  std::cout << qPrintable(tr("Arguments:")) << std::endl;
  std::cout << qPrintable(QStringLiteral("  %1  %2").arg(tr("paths"), 4 - columnWidth).arg(tr("File(s) to open (optional)."))) << std::endl;
}

static QStringList lineWrap(QString line, int width)
{
  QStringList lines;
  while (true) {
    int spacePos = line.lastIndexOf(' ', width);
    if (spacePos < 0) {
      lines << line;
      return lines;
    }
    lines << line.left(spacePos);
    line = line.mid(spacePos + 1);
  }
}

void DLApplication::showHelp(const QCommandLineOption& opt, int columnWidth, int lineWidth) const
{
  if (opt.flags() & QCommandLineOption::HiddenFromHelp) {
    return;
  }

  QString leftColumn;
  for (const QString& name : opt.names()) {
    leftColumn += leftColumn.isEmpty() ? "  " : ", ";
#ifdef Q_OS_WIN
    leftColumn += "/";
#else
    if (name.length() > 1) {
      leftColumn += "--";
    } else {
      leftColumn += "-";
    }
#endif
    leftColumn += name;
  }

  QStringList wrapped = lineWrap(opt.description(), lineWidth - columnWidth);
  bool first = true;
  for (const QString& line : wrapped) {
    if (first) {
      std::cerr << qPrintable(QStringLiteral("%1%2").arg(leftColumn, -columnWidth).arg(line)) << std::endl;
      first = false;
    } else {
      std::cerr << qPrintable(QString(columnWidth, ' ')) << qPrintable(line) << std::endl;
    }
  }
}

void DLApplication::showVersion() const
{
  std::cout << qPrintable(tr("Dreamline version ")) << qPrintable(QCoreApplication::applicationVersion()) << std::endl;
}

void DLApplication::showUnknownError(const QString& optName) const
{
  std::cerr << qPrintable(tr("%1: Unknown option \"%2\"").arg(m_args[0]).arg(optName)) << std::endl;
}

void DLApplication::showUnexpectedValueError(const QString& optName) const
{
  std::cerr << qPrintable(tr("%1: Option \"%2\" does not accept a value").arg(m_args[0]).arg(optName)) << std::endl;
}

void DLApplication::showMissingValueError(const QString& optName) const
{
  std::cerr << qPrintable(tr("%1: Option \"%2\" requires a value").arg(m_args[0]).arg(optName)) << std::endl;
}
