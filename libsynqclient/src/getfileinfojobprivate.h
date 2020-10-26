/*
 * Copyright 2020 Martin Hoeher <martin@rpdev.net>
 *
 * This file is part of SynqClient.
 *
 * SynqClient is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * SynqClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SynqClient.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYNQCLIENT_GETFILEINFOJOBPRIVATE_H
#define SYNQCLIENT_GETFILEINFOJOBPRIVATE_H

#include <QString>
#include <QVariantMap>

#include "getfileinfojob.h"
#include "abstractjobprivate.h"

namespace SynqClient {

class GetFileInfoJobPrivate : public AbstractJobPrivate
{
public:
    explicit GetFileInfoJobPrivate(GetFileInfoJob* q);

    Q_DECLARE_PUBLIC(GetFileInfoJob);

    QString path;
    QVariantMap fileInfo;
};

} // namespace SynqClient

#endif // SYNQCLIENT_GETFILEINFOJOBPRIVATE_H
