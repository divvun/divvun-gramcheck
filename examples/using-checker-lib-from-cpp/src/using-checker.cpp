/*
* Copyright (C) 2017–2019, Kevin Brubeck Unhammer <unhammer@fsfe.org>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// IMPORTANT: Check any changes to this file into git before running make check!

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdlib.h>

#include <locale>
#include <codecvt>

#include <divvun/checker.hpp>
#include "cxxopts.hpp"


int runXml(const std::string& specpath, const std::u16string& pipename, bool verbose) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        const auto& spec = std::unique_ptr<divvun::CheckerSpec>(new divvun::CheckerSpec(specpath));
	if(!spec->hasPipe(utf16conv.to_bytes(pipename))) {
		std::cerr << "ERROR: Couldn't find pipe " << utf16conv.to_bytes(pipename) << " in " << specpath << std::endl;
		return EXIT_FAILURE;
	}
        const auto& pipeline = spec->getChecker(utf16conv.to_bytes(pipename), verbose);
	for (std::string line; std::getline(std::cin, line);) {
		std::stringstream pipe_in(line);
		std::stringstream pipe_out;
		pipeline->proc(pipe_in, pipe_out);
		std::cout << pipe_out.str() << std::endl;
	}
        std::cout << "done" << std::endl;
	return EXIT_SUCCESS;
}

int printNamesXml(const std::string& path, bool verbose) {
	const auto& spec = divvun::CheckerSpec(path);
	std::cout << "Please specify a pipeline variant with the -n/--variant option. Available variants in pipespec:" << std::endl;
	for(const auto& p : spec.pipeNames()) {
		std::cout << p.c_str() << std::endl;
	}
	return EXIT_SUCCESS;
}

int runAr(const std::string& path, const std::u16string& pipename, bool verbose) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        const auto& ar_spec = std::unique_ptr<divvun::ArCheckerSpec>(new divvun::ArCheckerSpec(path));
        const auto& ar_pipeline = ar_spec->getChecker(utf16conv.to_bytes(pipename), verbose);
	for (std::string line; std::getline(std::cin, line);) {
		std::stringstream pipe_in(line);
		std::stringstream pipe_out;
		ar_pipeline->proc(pipe_in, pipe_out);
		std::cout << pipe_out.str() << std::endl;
		std::stringstream pipe_in2(line);
		const auto& errs = ar_pipeline->proc_errs(pipe_in2);
		for(const auto& e : errs) {
			std::cout << "form=" << utf16conv.to_bytes(e.form)
				  << " beg=" << e.beg
				  << " end=" << e.end
				  << " err=" << utf16conv.to_bytes(e.err)
				  << " msg=" << utf16conv.to_bytes(e.msg.first)
				  << " dsc=" << utf16conv.to_bytes(e.msg.second);
			for(const auto& r : e.rep) {
				std::cout << " rep=" << utf16conv.to_bytes(r);
			}
			std::cout  << std::endl;
		}
	}
	return EXIT_SUCCESS;
}

int printNamesAr(const std::string& path, bool verbose) {
	const auto& ar_spec = divvun::ArCheckerSpec(path);
	std::cout << "Please specify a pipeline variant with the -n/--variant option. Available variants in archive:" << std::endl;
	for(const auto& p : ar_spec.pipeNames()) {
		const auto& name = p.c_str();
		std::cout << name << std::endl;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char ** argv)
{
	try
	{
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
		cxxopts::Options options(argv[0], "BIN - generate grammar checker suggestions from a CG stream");

		options.add_options()
			("s,spec", "Pipeline XML specification", cxxopts::value<std::string>(), "FILE")
			("a,archive", "Zipped pipeline archive of language data", cxxopts::value<std::string>(), "FILE")
			("n,variant", "Name of the pipeline variant", cxxopts::value<std::string>(), "NAME")
			("i,input", "Input file (UNIMPLEMENTED, stdin for now)", cxxopts::value<std::string>(), "FILE")
			("o,output", "Output file (UNIMPLEMENTED, stdout for now)", cxxopts::value<std::string>(), "FILE")
			("z,null-flush", "(Ignored, we always flush on <STREAMCMD:FLUSH>, outputting \\0 if --json).")
			("v,verbose", "Be verbose")
			("h,help", "Print help")
			;

		std::vector<std::string> pos = {
			"spec",
			"archive",
			"variant"
			//"input"
			//"output"
		};
		options.parse_positional(pos);
		options.parse(argc, argv);

		if(argc > 1) {
			std::cout << options.help({""}) << std::endl;
			std::cerr << "ERROR: got " << argc-1+pos.size() <<" arguments; expected only " << pos.size() << std::endl;
			return EXIT_SUCCESS;
		}

		if (options.count("help"))
		{
			std::cout << options.help({""}) << std::endl;
			return EXIT_SUCCESS;
		}
		bool verbose = options.count("v");

		if(options.count("spec")) {
			const auto& specfile = options["spec"].as<std::string>();
			if(verbose) {
				std::cerr << "Reading specfile " << specfile << std::endl;
			}
			if(!options.count("variant")) {
				return printNamesXml(specfile, verbose);
			}
			else {
				const auto& pipename = utf16conv.from_bytes(options["variant"].as<std::string>());
				return runXml(specfile, pipename, verbose);
			}
		}
		else if(options.count("archive")) {
			const auto& archive = options["archive"].as<std::string>();
			if(verbose) {
				std::cerr << "Reading zipped archive file " << archive << std::endl;
			}
			if(!options.count("variant")) {
				return printNamesAr(archive, verbose);
			}
			else {
				const auto& pipename = utf16conv.from_bytes(options["variant"].as<std::string>());
				return runAr(archive, pipename, verbose);
			}
		}
		else {
			std::cerr << "ERROR: Pipespec file required" << std::endl;
			return EXIT_FAILURE;
		}

	}
	catch (const cxxopts::OptionException& e)
	{
		std::cerr << "ERROR: couldn't parse options: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
