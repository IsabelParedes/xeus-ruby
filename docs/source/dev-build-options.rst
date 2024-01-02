..  Copyright (c) 2024,    

   Distributed under the terms of the BSD 3-Clause License.  

   The full license is in the file LICENSE, distributed with this software.

Build and configuration
=======================

General Build Options
---------------------

Building the xeus-ruby library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``xeus-ruby`` build supports the following options:

- ``XEUS_RUBY_BUILD_SHARED``: Build the ``xeus-ruby`` shared library. **Enabled by default**.
- ``XEUS_RUBY_BUILD_STATIC``: Build the ``xeus-ruby`` static library. **Enabled by default**.


- ``XEUS_RUBY_USE_SHARED_XEUS``: Link with a `xeus` shared library (instead of the static library). **Enabled by default**.

Building the kernel
~~~~~~~~~~~~~~~~~~~

The package includes two options for producing a kernel: an executable ``xruby`` and a Python extension module, which is used to launch a kernel from Python.

- ``XEUS_RUBY_BUILD_EXECUTABLE``: Build the ``xruby``  executable. **Enabled by default**.


If ``XEUS_RUBY_USE_SHARED_XEUS_RUBY`` is disabled, xruby  will be linked statically with ``xeus-ruby``.

Building the Tests
~~~~~~~~~~~~~~~~~~

- ``XEUS_RUBY_BUILD_TESTS ``: enables the tets  **Disabled by default**.

