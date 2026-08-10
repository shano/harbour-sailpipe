#ifndef YTDLPPROCESS_H
#define YTDLPPROCESS_H

#include <QString>
#include <QStringList>
#include <QJsonDocument>

class YtDlpProcess
{
public:
  struct Result {
    bool success;
    QJsonDocument output;
    QString errorMessage;
  };

  static Result run(QStringList const& args, int timeoutMs = 30000);
};

#endif // YTDLPPROCESS_H
