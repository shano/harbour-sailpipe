#include <QProcess>
#include <QFileInfo>

#include "ytdlpmanager.h"
#include "ytdlpprocess.h"

YtDlpProcess::Result YtDlpProcess::run(QStringList const& args, int timeoutMs)
{
  Result result{false, QJsonDocument(), QString()};

  QString binary = YtDlpManager::binaryPath();
  if (!QFileInfo(binary).isExecutable()) {
    result.errorMessage = QStringLiteral("yt-dlp is not installed");
    return result;
  }

  QProcess process;
  process.start(binary, args);

  if (!process.waitForFinished(timeoutMs)) {
    process.kill();
    result.errorMessage = QStringLiteral("yt-dlp timed out");
    return result;
  }

  if (process.exitCode() != 0) {
    QString stderrOutput = QString::fromUtf8(process.readAllStandardError());
    QStringList lines = stderrOutput.split(QChar('\n'), QString::SkipEmptyParts);
    result.errorMessage = lines.isEmpty() ? QStringLiteral("yt-dlp failed") : lines.last();
    return result;
  }

  QByteArray stdOut = process.readAllStandardOutput();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(stdOut, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    result.errorMessage = QString("Failed to parse yt-dlp output: %1").arg(parseError.errorString());
    return result;
  }

  result.success = true;
  result.output = doc;
  return result;
}
