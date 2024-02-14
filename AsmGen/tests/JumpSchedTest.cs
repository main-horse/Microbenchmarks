﻿using System.Text;

namespace AsmGen
{
    public class JumpSchedTest : UarchTest
    {
        public JumpSchedTest(int low, int high, int step)
        {
            this.Counts = UarchTestHelpers.GenerateCountArray(low, high, step);
            this.Prefix = "jumpsched";
            this.Description = "Scheduler, Not-Taken Jumps";
            this.FunctionDefinitionParameters = "uint64_t iterations, int *arr";
            this.GetFunctionCallParameters = "structIterations, A";
            this.DivideTimeByCount = false;
        }

        public override bool SupportsIsa(IUarchTest.ISA isa)
        {
            if (isa == IUarchTest.ISA.amd64) return true;
            if (isa == IUarchTest.ISA.aarch64) return true;
            // if (isa == IUarchTest.ISA.mips64) return true;
            // if (isa == IUarchTest.ISA.riscv) return true;
            return false;
        }

        public override void GenerateAsm(StringBuilder sb, IUarchTest.ISA isa)
        {
            if (isa == IUarchTest.ISA.amd64)
            {
                string[] unrolledJumps = new string[1];
                unrolledJumps[0] = "  cmp %rdi, %rsi\n  je jumpsched_reallybadthing";
                UarchTestHelpers.GenerateX86AsmStructureTestFuncs(sb, this.Counts, this.Prefix, unrolledJumps, unrolledJumps, includePtrChasingLoads: true);

                sb.AppendLine("jumpsched_reallybadthing:\n  int3");
            }
            else if (isa == IUarchTest.ISA.aarch64)
            {
                string[] unrolledJumps = new string[1];
                unrolledJumps[0] = "  cmp x25, x26\n  b.eq jumpsched_reallybadthing";
                UarchTestHelpers.GenerateArmAsmStructureTestFuncs(sb, this.Counts, this.Prefix, unrolledJumps, unrolledJumps, includePtrChasingLoads: true);
                sb.AppendLine("jumpsched_reallybadthing:\n  .word 0xf7f0a000");
            }
            else if (isa == IUarchTest.ISA.riscv)
            {
                // todo
                string[] unrolledAdds = new string[4];
                unrolledAdds[0] = "  mul x30, x30, x5";
                unrolledAdds[1] = "  mul x29, x29, x5";
                unrolledAdds[2] = "  mul x28, x28, x5";
                unrolledAdds[3] = "  mul x31, x31, x5";

                string[] unrolledAdds1 = new string[4];
                unrolledAdds1[0] = "  mul x30, x30, x6";
                unrolledAdds1[1] = "  mul x31, x31, x6";
                unrolledAdds1[2] = "  mul x28, x28, x6";
                unrolledAdds1[3] = "  mul x29, x29, x6";
                UarchTestHelpers.GenerateRiscvAsmStructureTestFuncs(sb, this.Counts, this.Prefix, unrolledAdds, unrolledAdds1, false);
            }
        }
    }
}
