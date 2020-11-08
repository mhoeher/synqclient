#include "abstractjobprivate.h"

namespace SynqClient {

AbstractJobPrivate::AbstractJobPrivate(AbstractJob* q)
    : q_ptr(q), error(JobError::NoError), errorString(), state(JobState::Ready)
{
}

AbstractJobPrivate::~AbstractJobPrivate() {}

} // namespace SynqClient
