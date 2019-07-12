// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef CALLBACK_H
#define CALLBACK_H

/// Callback interface
/// Application need to implement the different kinds of callbacks based this interface,
/// like receive callback, send callback, read callback, write callback...
class Callback {
  public:
    virtual ~Callback() = default;
    /// \Param_1, \param_2 can be whatever you want, like connection, buffer size, buffer id.
    /// Please see more examples in example directory.
    virtual void operator()(void *param_1, void *param_2) = 0;
};

#endif
