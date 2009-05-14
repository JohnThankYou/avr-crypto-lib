# Makefile for SHA256
ALGO_NAME := SHA256

# comment out the following line for removement of SHA256 from the build process
HASHES += $(ALGO_NAME)

$(ALGO_NAME)_OBJ      := sha256-asm.o
$(ALGO_NAME)_TEST_BIN := main-sha256-test.o dump.o hfal_sha256.o $(CLI_STD) $(HFAL_STD)
                       
			
$(ALGO_NAME)_NESSIE_TEST      := "nessie"
$(ALGO_NAME)_PERFORMANCE_TEST := "performance"

