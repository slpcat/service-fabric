// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

#include "stdafx.h"

#include <boost/test/unit_test.hpp>
#include "Common/boost-taef.h"
#include "TStoreTestBase.h"

namespace TStoreTests
{
   using namespace ktl;

   class ConsolidationTest : public TStoreTestBase<int, int, IntComparer, TestStateSerializer<int>, TestStateSerializer<int>>
   {
   public:
      ConsolidationTest()
      {
         Setup(3);
      }

      ~ConsolidationTest()
      {
         Cleanup();
      }

      //CommonConfig config; // load the config object as its needed for the tracing to work

#pragma region test functions
    public:
        ktl::Awaitable<void> Add_ConsolidateRead_ShouldSucceed_Test()
       {
          {
             WriteTransaction<int, int>::SPtr tx1 = CreateWriteTransaction();
             for (int i = 0; i < 10; i++)
             {
                co_await Store->AddAsync(*tx1->StoreTransactionSPtr, i, i, DefaultTimeout, CancellationToken::None);
             }

             co_await tx1->CommitAsync();
          }

          // Checkpoint
          co_await CheckpointAsync();

          {
             for (ULONG m = 0; m < Stores->Count(); m++)
             {
                WriteTransaction<int, int>::SPtr tx2 = CreateWriteTransaction(*(*Stores)[m]);
                for (int i = 0; i < 10; i++)
                {
                   co_await VerifyKeyExistsAsync(*(*Stores)[m], *tx2->StoreTransactionSPtr, i, -1, i);
                }

                co_await tx2->AbortAsync();
             }
          }

          co_await CloseAndReOpenStoreAsync();

          for (int i = 0; i < 10; i++)
          {
             co_await VerifyKeyExistsInStoresAsync(i, -1, i);
          }
           co_return;
       }

        ktl::Awaitable<void> AddConsolidateRead_UpdateRead_ShouldSucceed_Test()
       {
          // 1. Add.
          {
             WriteTransaction<int, int>::SPtr tx1 = CreateWriteTransaction();

             for (int i = 0; i < 10; i++)
             {
                co_await Store->AddAsync(*tx1->StoreTransactionSPtr, i, i, DefaultTimeout, CancellationToken::None);
             }

             co_await tx1->CommitAsync();
          }

          // 2. Checkpoint.
          co_await CheckpointAsync();

          {
             for (ULONG m = 0; m < Stores->Count(); m++)
             {
                WriteTransaction<int, int>::SPtr tx2 = CreateWriteTransaction(*(*Stores)[m]);
                for (int i = 0; i < 10; i++)
                {
                   co_await VerifyKeyExistsAsync(*(*Stores)[m], *tx2->StoreTransactionSPtr, i, -1, i);
                }

                co_await tx2->AbortAsync();
             }
          }

          // 3. Update.
          {
             WriteTransaction<int, int>::SPtr tx3 = CreateWriteTransaction();
             for (int i = 0; i < 10; i++)
             {
                co_await Store->ConditionalUpdateAsync(*tx3->StoreTransactionSPtr, i, i + 10, DefaultTimeout, CancellationToken::None);
             }

             co_await tx3->CommitAsync();
          }

          {
             for (ULONG m = 0; m < Stores->Count(); m++)
             {
                WriteTransaction<int, int>::SPtr tx4 = CreateWriteTransaction(*(*Stores)[m]);
                for (int i = 0; i < 10; i++)
                {
                   co_await VerifyKeyExistsAsync(*(*Stores)[m], *tx4->StoreTransactionSPtr, i, -1, i + 10);
                }

                co_await tx4->AbortAsync();
             }
          }

          // 4. Checkpoint again.

          co_await CheckpointAsync();

          // 5. Reopen and verify.
          co_await CloseAndReOpenStoreAsync();

          for (int i = 0; i < 10; i++)
          {
             co_await VerifyKeyExistsInStoresAsync(i, -1, i + 10);
          }
           co_return;
       }

        ktl::Awaitable<void> AddConsolidateRead_UpdateConsolidateRead_ShouldSucceed_Test()
       {
          // 1. Add.
          {
             WriteTransaction<int, int>::SPtr tx1 = CreateWriteTransaction();

             for (int i = 0; i < 10; i++)
             {
                co_await Store->AddAsync(*tx1->StoreTransactionSPtr, i, i, DefaultTimeout, CancellationToken::None);
             }

             co_await tx1->CommitAsync();
          }

          // 2. Checkpoint.
          co_await CheckpointAsync();

          {
             for (ULONG m = 0; m < Stores->Count(); m++)
             {
                WriteTransaction<int, int>::SPtr tx2 = CreateWriteTransaction(*(*Stores)[m]);
                for (int i = 0; i < 10; i++)
                {
                   co_await VerifyKeyExistsAsync(*(*Stores)[m], *tx2->StoreTransactionSPtr, i, -1, i);
                }

                co_await tx2->AbortAsync();
             }
          }

          // 3. Update.
          {
             WriteTransaction<int, int>::SPtr tx3 = CreateWriteTransaction();
             for (int i = 0; i < 10; i++)
             {
                co_await Store->ConditionalUpdateAsync(*tx3->StoreTransactionSPtr, i, i + 10, DefaultTimeout, CancellationToken::None);
             }

             co_await tx3->CommitAsync();
          }

          // 5. Checkpoint.
          co_await CheckpointAsync();

          {
             for (ULONG m = 0; m < Stores->Count(); m++)
             {
                WriteTransaction<int, int>::SPtr tx4 = CreateWriteTransaction(*(*Stores)[m]);
                for (int i = 0; i < 10; i++)
                {
                   co_await VerifyKeyExistsAsync(*(*Stores)[m], *tx4->StoreTransactionSPtr, i, -1, i + 10);
                }

                co_await tx4->AbortAsync();
             }
          }

          // 6. Reopen and verify.
          co_await CloseAndReOpenStoreAsync();

          for (int i = 0; i < 10; i++)
          {
             co_await VerifyKeyExistsInStoresAsync(i, -1, i + 10);
          }
           co_return;
       }

        ktl::Awaitable<void> AddCheckpoint_ReOpenUpdate_ShouldSucceed_Test()
       {
          // 1. Add.
          {
             WriteTransaction<int, int>::SPtr tx1 = CreateWriteTransaction();
             for (int i = 0; i < 10; i++)
             {
                co_await Store->AddAsync(*tx1->StoreTransactionSPtr, i, i, DefaultTimeout, CancellationToken::None);
             }

             co_await tx1->CommitAsync();
          }

          // 2. Checkpoint.
          co_await CheckpointAsync();

          {
             for (ULONG m = 0; m < Stores->Count(); m++)
             {
                WriteTransaction<int, int>::SPtr tx2 = CreateWriteTransaction(*(*Stores)[m]);
                for (int i = 0; i < 10; i++)
                {
                   co_await VerifyKeyExistsAsync(*(*Stores)[m], *tx2->StoreTransactionSPtr, i, -1, i);
                }

                co_await tx2->AbortAsync();
             }
          }

          // 3. ReOpen.
          co_await CloseAndReOpenStoreAsync();

          // 4. Update.
          {
             WriteTransaction<int, int>::SPtr tx3 = CreateWriteTransaction();
             for (int i = 0; i < 10; i++)
             {
                co_await Store->ConditionalUpdateAsync(*tx3->StoreTransactionSPtr, i, i + 10, DefaultTimeout, CancellationToken::None);
             }

             co_await tx3->CommitAsync();
          }

          for (int i = 0; i < 10; i++)
          {
             co_await VerifyKeyExistsInStoresAsync(i, -1, i + 10);
          }
           co_return;
       }

        ktl::Awaitable<void> Add_Checkpoint_Commit_Test()
       {
          int key1 = 5;
          int key2 = 7;
          int value = 10;

          // 1. Add.
          WriteTransaction<int, int>::SPtr tx1 = CreateWriteTransaction();
          co_await Store->AddAsync(*tx1->StoreTransactionSPtr, key1, value, DefaultTimeout, CancellationToken::None);
          co_await Store->AddAsync(*tx1->StoreTransactionSPtr, key2, value, DefaultTimeout, CancellationToken::None);

          // 2. Checkpoint.
          co_await CheckpointAsync();

          // 3. Apply
          co_await tx1->CommitAsync();

          // 4. Dispose
          tx1 = nullptr;

          co_await VerifyKeyExistsInStoresAsync(key1, -1, value); 
          co_await VerifyKeyExistsInStoresAsync(key2, -1, value);
           co_return;
       }
    #pragma endregion
   };

   BOOST_FIXTURE_TEST_SUITE(ConsolidationTestSuite, ConsolidationTest)

      BOOST_AUTO_TEST_CASE(Add_ConsolidateRead_ShouldSucceed)
   {
       SyncAwait(Add_ConsolidateRead_ShouldSucceed_Test());
   }

   BOOST_AUTO_TEST_CASE(AddConsolidateRead_UpdateRead_ShouldSucceed)
   {
       SyncAwait(AddConsolidateRead_UpdateRead_ShouldSucceed_Test());
   }

   BOOST_AUTO_TEST_CASE(AddConsolidateRead_UpdateConsolidateRead_ShouldSucceed)
   {
       SyncAwait(AddConsolidateRead_UpdateConsolidateRead_ShouldSucceed_Test());
   }

   BOOST_AUTO_TEST_CASE(AddCheckpoint_ReOpenUpdate_ShouldSucceed)
   {
       SyncAwait(AddCheckpoint_ReOpenUpdate_ShouldSucceed_Test());
   }


   BOOST_AUTO_TEST_CASE(Add_Checkpoint_Commit)
   {
       SyncAwait(Add_Checkpoint_Commit_Test());
   }

   BOOST_AUTO_TEST_SUITE_END()
}
