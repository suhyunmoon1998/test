TESTS_32I := $(filter-out rv32ui-p-ma_data.hex rv32ui-p-fence_i.hex, $(wildcard rv32ui-p-*.hex))

all:

run:
	$(foreach test, $(TESTS_32I), ../tinyrv $(test) || exit;)

run-o:
	$(foreach test, $(TESTS_32I), ../tinyrv -o $(test) || exit;)

run-g:
	$(foreach test, $(TESTS_32I), ../tinyrv -g $(test) || exit;)
	
run-og:
	$(foreach test, $(TESTS_32I), ../tinyrv -og $(test) || exit;)

clean: