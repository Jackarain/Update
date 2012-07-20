//
// updater.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011 Jack (jack.wgm@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// * $Id: updater.hpp 49 2011-07-15 03:00:34Z jack $
//

#ifndef __UPDATER_H__
#define __UPDATER_H__

#include <boost/function.hpp>
class updater_impl;

#pragma once


class updater
{
public:
	typedef boost::function<void (std::string file, 
		int count, int index)> fun_check_files_callback;
	typedef boost::function<void (std::string file, 
		int count, int index)> fun_update_files_process;
	typedef boost::function<void (std::string file, int count, int index, 
		int total_size, int total_read_bytes,
		int file_size, int read_bytes)> fun_down_load_callback;

public:
	updater();
	~updater();

public:
	bool start(const std::string& url, fun_down_load_callback dl, 
		fun_check_files_callback cf, fun_update_files_process uf, std::string setup_path);
	void stop();
	void pause();
	void resume();

private:
	updater_impl *m_updater;
};

#endif // __UPDATER_H__
