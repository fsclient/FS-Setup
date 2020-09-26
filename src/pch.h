#pragma once

// Precompiled headers

// STD stuff
#include <optional>
#include <thread>
#include <chrono>

// net
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

// Windows
#include <windows.h>
#include <CommCtrl.h>
#include <appmodel.h>

// boost

#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include "boost/nowide/convert.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/hex.hpp"
#include "boost/format.hpp"

// Additional dependencies

#include "tinyxml2.h"