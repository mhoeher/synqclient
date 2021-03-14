#ifndef SYNQCLIENT_ABSTRACTJOBPRIVATE_H
#define SYNQCLIENT_ABSTRACTJOBPRIVATE_H

#include <QString>
#include <QtGlobal>

#include "SynqClient/abstractjob.h"

namespace SynqClient {

class AbstractJobPrivate
{
public:
    explicit AbstractJobPrivate(AbstractJob* q);
    virtual ~AbstractJobPrivate();

    AbstractJob* q_ptr;
    Q_DECLARE_PUBLIC(AbstractJob);

    JobError error;
    QString errorString;
    JobState state;
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTJOBPRIVATE_H
