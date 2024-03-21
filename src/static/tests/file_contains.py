#
# Copyright (c) 2020-2024 Bull S.A.S. All rights reserved.
# Copyright (c) 2020-2024 Inria All rights reserved.
#

# This file is part of Parcoach RMA
# Parcoach RMA is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
# Parcoach RMA is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with Parcoach RMA.
# If not, see <https://www.gnu.org/licenses/>.


import sys
import argparse

parser = argparse.ArgumentParser(description='Check that the first input file contains all lines from the second input files. Each expectation must be unique.')
parser.add_argument('input', help='the input file to check')
parser.add_argument('expectation', help='the expectation file, one per line')
args = parser.parse_args()

expectations = set(l for l in open(args.expectation))
detected = set(l for l in open(args.input) if l.startswith('PARCOACH'))

missed = expectations - detected
extra = detected - expectations

n_missed = len(missed)
n_extra = len(extra)

if n_missed > 0:
  print("Some expectations ({}) were missed:".format(n_missed))
  for m in missed:
    print(m)
if n_extra > 0:
  print("Some diagnostics ({}) were detected and unexpected:".format(n_extra))
  for m in extra:
    print(m)

sys.exit(n_missed + n_extra)
