# UMANS: Unified Microscopic Agent Navigation Simulator
# MIT License
# Copyright (C) 2018-2020  Inria Rennes Bretagne Atlantique - Rainbow - Julien Pettré
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# === Get Git information
## Extra info here : http://xit0.org/2013/04/cmake-use-git-branch-and-commit-details-in-project/

cmake_minimum_required( VERSION 3.10 )

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  # Get the current working branch
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  # Get the latest commit hash
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  # Get the git tag version the commit is associated to
  execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_TAG_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Get the remote name of current commit
  execute_process(
    COMMAND git rev-parse --abbrev-ref --symbolic-full-name @{u}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_REMOTE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Get the remote URL of current commit
  execute_process(
    COMMAND git ls-remote --get-url
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_REMOTE_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(GIT_BRANCH "")
  set(GIT_COMMIT_HASH "")
  set(GIT_COMMIT_REMOTE "")
  set(GIT_COMMIT_REMOTE_URL "")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")


# Get full date of current build generation
set(BUILD_DATE "")
string(TIMESTAMP BUILD_DATE)

# Get year of current build generation
set(BUILD_DATE_YEAR "")
string(TIMESTAMP BUILD_DATE_YEAR %Y)


# Debug message

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    message(STATUS "A git projet as been found in: ${CMAKE_SOURCE_DIR}")
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    message(STATUS "Not git projet as been found in: ${CMAKE_SOURCE_DIR}")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	
message(STATUS "Git current branch: ${GIT_BRANCH}")
message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
message(STATUS "Git tag version: ${GIT_COMMIT_TAG_VERSION}")
message(STATUS "Git commit remote: ${GIT_COMMIT_REMOTE}")
message(STATUS "Git commit remote URL: ${GIT_COMMIT_REMOTE_URL}")
message(STATUS "Makefile generated on ${BUILD_DATE} on year ${BUILD_DATE_YEAR}")

# Generate version.h header

message(STATUS "Generating version.h in ${CMAKE_BINARY_DIR}/generated/")

configure_file(
  ${CMAKE_SOURCE_DIR}/src/Include/version.h.in
  ${CMAKE_BINARY_DIR}/generated/version.h
)

include_directories(${CMAKE_BINARY_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR}/src/Include)

#add_subdirectory(src)