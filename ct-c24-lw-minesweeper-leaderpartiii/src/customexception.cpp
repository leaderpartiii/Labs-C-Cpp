#include "include/customexception.h"

#include <QException>
CustomException::CustomException(const QString &message) : QException(), message_(message) {}
