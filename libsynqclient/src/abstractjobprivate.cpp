#include "abstractjobprivate.h"

#include <QNetworkRequest>

namespace SynqClient {

AbstractJobPrivate::AbstractJobPrivate(AbstractJob* q)
    : q_ptr(q),
      error(JobError::NoError),
      errorString(),
      state(JobState::Ready),
      transferTimeout(QNetworkRequest::DefaultTransferTimeoutConstant)
{
}

AbstractJobPrivate::~AbstractJobPrivate() {}

} // namespace SynqClient
