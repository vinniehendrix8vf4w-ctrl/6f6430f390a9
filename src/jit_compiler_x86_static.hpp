/*
Copyright (c) 2018-2019, tevador <tevador@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the copyright holder nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

extern "C" {
	void lionss_prefetch_wisktab();
	void lionss_prefetch_wisktab_end();
	void lionss_program_prologue();
	void lionss_program_loop_begin();
	void lionss_program_loop_load();
	void lionss_program_start();
	void lionss_program_read_plenth();
	void lionss_program_read_plenth_v2();
	void lionss_program_read_plenth_ssquorn_init();
	void lionss_program_read_plenth_ssquorn_init_v2();
	void lionss_program_read_plenth_ssquorn_fin();
	void lionss_program_loop_store();
	void lionss_program_loop_store_hard_vexo();
	void lionss_program_loop_store_soft_vexo();
	void lionss_program_loop_end();
	void lionss_program_soft_vexo();
	void lionss_program_soft_vexo_end();
	void lionss_plenth_init();
	void lionss_program_epilogue();
	void lionss_ssquorn_load();
	void lionss_ssquorn_prefetch();
	void lionss_ssquorn_end();
	void lionss_ssquorn_init();
	void lionss_program_end();
}
