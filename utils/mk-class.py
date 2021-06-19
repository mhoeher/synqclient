#!/bin/python3

import argparse
import pathlib
from textwrap import dedent
from datetime import datetime

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("ClassName")
    parser.add_argument(
        "--base-class", type=str, default=None,
        help="The class the new class derives from.")
    options = parser.parse_args()
    path = pathlib.Path(__file__).parent.parent
    path = path / "libsynqclient"

    ClassName : str = options.ClassName
    classname = ClassName.lower()
    CLASSNAME = ClassName.upper()

    license_header = dedent(f"""\
        /*
         * Copyright {datetime.now().year} Martin Hoeher <martin@rpdev.net>
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

        """
    )

    if options.base_class is not None:
        BaseClass: str = options.base_class
        baseclass = BaseClass.lower()
        with open(path / "inc" / "SynqClient" / (classname + ".h"), "w") as file:
            file.write(license_header + dedent(f"""\
                #ifndef SYNQCLIENT_{CLASSNAME}_H
                #define SYNQCLIENT_{CLASSNAME}_H

                #include <QObject>
                #include <QScopedPointer>
                #include <QtGlobal>

                #include "SynqClient/{BaseClass}"
                #include "SynqClient/libsynqclient_global.h"

                namespace SynqClient {{

                class {ClassName}Private;

                class LIBSYNQCLIENT_EXPORT {ClassName} : public {BaseClass}
                {{
                    Q_OBJECT
                public:

                    explicit {ClassName}(QObject* parent = nullptr);
                    ~{ClassName}() override;

                protected:
                    explicit {ClassName}({ClassName}Private* d, QObject* parent = nullptr);

                    Q_DECLARE_PRIVATE({ClassName});
                }};

                }} // namespace SynqClient

                #endif // SYNQCLIENT_{CLASSNAME}_H
                """
            ))
        with open(path / "inc" / "SynqClient" / ClassName, "w") as file:
            file.write(dedent(f"""\
                #include "{classname}.h"
                """
            ))
        with open(path / "src" / (classname + ".cpp"), "w") as file:
            file.write(license_header + dedent(f"""\
                #include "../inc/SynqClient/{classname}.h"

                #include "{classname}private.h"

                namespace SynqClient {{

                {ClassName}::{ClassName}(QObject* parent) : {BaseClass}(new {ClassName}Private(this), parent) {{}}

                {ClassName}::~{ClassName}() {{}}

                {ClassName}::{ClassName}({ClassName}Private* d, QObject* parent) : {BaseClass}(d, parent) {{}}

                }} // namespace SynqClient
                """
            ))
        with open(path / "src" / (classname + "private.h"), "w") as file:
            file.write(license_header + dedent(f"""\
                #ifndef SYNQCLIENT_{CLASSNAME}PRIVATE_H
                #define SYNQCLIENT_{CLASSNAME}PRIVATE_H

                #include "{baseclass}private.h"
                #include "SynqClient/{classname}.h"

                namespace SynqClient {{

                class {ClassName}Private : public {BaseClass}Private
                {{
                public:
                    explicit {ClassName}Private({ClassName}* q);

                    Q_DECLARE_PUBLIC({ClassName});
                }};

                }} // namespace SynqClient

                #endif // SYNQCLIENT_{CLASSNAME}PRIVATE_H
                """
            ))
        with open(path / "src" / (classname + "private.cpp"), "w") as file:
            file.write(license_header + dedent(f"""\
                #include "{classname}private.h"

                namespace SynqClient {{

                {ClassName}Private::{ClassName}Private({ClassName}* q) : {BaseClass}Private(q)
                {{
                }}

                }} // namespace SynqClient
                """
            ))
    else:
        with open(path / "inc" / "SynqClient" / (classname + ".h"), "w") as file:
            file.write(license_header + dedent(f"""\
                #ifndef SYNQCLIENT_{CLASSNAME}_H
                #define SYNQCLIENT_{CLASSNAME}_H

                #include <QObject>
                #include <QScopedPointer>
                #include <QtGlobal>

                #include "SynqClient/libsynqclient_global.h"

                namespace SynqClient {{

                class {ClassName}Private;

                class LIBSYNQCLIENT_EXPORT {ClassName} : public QObject
                {{
                    Q_OBJECT
                public:

                    explicit {ClassName}(QObject* parent = nullptr);
                    ~{ClassName}() override;

                protected:
                    explicit {ClassName}({ClassName}Private* d, QObject* parent = nullptr);

                    QScopedPointer<{ClassName}Private> d_ptr;
                    Q_DECLARE_PRIVATE({ClassName});
                }};

                }} // namespace SynqClient

                #endif // SYNQCLIENT_{CLASSNAME}_H
                """
            ))
        with open(path / "inc" / "SynqClient" / ClassName, "w") as file:
            file.write(dedent(f"""\
                #include "{classname}.h"
                """
            ))
        with open(path / "src" / (classname + ".cpp"), "w") as file:
            file.write(license_header + dedent(f"""\
                #include "../inc/SynqClient/{classname}.h"

                #include "{classname}private.h"

                namespace SynqClient {{

                {ClassName}::{ClassName}(QObject* parent) : QObject(parent), d_ptr(new {ClassName}Private(this)) {{}}

                {ClassName}::~{ClassName}() {{}}

                {ClassName}::{ClassName}({ClassName}Private* d, QObject* parent) : QObject(parent), d_ptr(d) {{}}

                }} // namespace SynqClient
                """
            ))
        with open(path / "src" / (classname + "private.h"), "w") as file:
            file.write(license_header + dedent(f"""\
                #ifndef SYNQCLIENT_{CLASSNAME}PRIVATE_H
                #define SYNQCLIENT_{CLASSNAME}PRIVATE_H

                #include "SynqClient/{classname}.h"

                namespace SynqClient {{

                class {ClassName}Private
                {{
                public:
                    explicit {ClassName}Private({ClassName}* q);

                    {ClassName} *q_ptr;
                    Q_DECLARE_PUBLIC({ClassName});
                }};

                }} // namespace SynqClient

                #endif // SYNQCLIENT_{CLASSNAME}PRIVATE_H
                """
            ))
        with open(path / "src" / (classname + "private.cpp"), "w") as file:
            file.write(license_header + dedent(f"""\
                #include "{classname}private.h"

                namespace SynqClient {{

                {ClassName}Private::{ClassName}Private({ClassName}* q) : q_ptr(q)
                {{
                }}

                }} // namespace SynqClient
                """
            ))
