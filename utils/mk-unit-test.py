#!/bin/python3

import argparse
import pathlib
from textwrap import dedent
from datetime import datetime

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("ClassName")
    options = parser.parse_args()
    path = pathlib.Path(__file__).parent.parent
    path = path / "tests"
    ClassName : str = options.ClassName
    classname = ClassName.lower()
    CLASSNAME = ClassName.upper()
    path = path / classname
    path.mkdir(exist_ok=True)
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
    with open(path / (classname + ".pro"), "w") as file:
        file.write(dedent(f"""\
            TESTNAME = {classname}
            include(../test.pri)
            """
        ))
    with open(path / "CMakeLists.txt", "w") as file:
        file.write(dedent(f"""\
            synqclient_add_test({classname})
            """
        ))
    with open(path / ("tst_" + classname + ".cpp"), "w") as file:
        file.write(dedent(f"""\
            #include <QtTest>

            // add necessary includes here
            #include "SynqClient/{ClassName}"

            class {ClassName}Test : public QObject
            {{
                Q_OBJECT

            public:
                {ClassName}Test();
                ~{ClassName}Test();

            private slots:
                void initTestCase();
                void cleanupTestCase();
            }};

            {ClassName}Test::{ClassName}Test() {{}}

            {ClassName}Test::~{ClassName}Test() {{}}

            void {ClassName}Test::initTestCase() {{}}

            void {ClassName}Test::cleanupTestCase() {{}}

            QTEST_MAIN({ClassName}Test)

            #include "tst_{classname}.moc"

            """
        ))
    