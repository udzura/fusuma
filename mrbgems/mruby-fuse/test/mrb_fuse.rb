##
## FUSE Test
##

assert("FUSE#hello") do
  t = FUSE.new "hello"
  assert_equal("hello", t.hello)
end
