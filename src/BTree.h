#pragma once


template <typename KeyType, typename ValueType, size_t M>
class BTree
{
   public:
      BTreeView()
         : m_root()
      {}

      void insert(const KeyType& key, const ValueType& value);
      RecordId find(const KeyType& key);
      void erase(const KeyType& key);

   private:
      template <typename KeyType, typename ValueType, size_t M>
      class BTreeNode
      {
         public:
            BTreeNode();
         private:
            size_t numKeys;
            std::array<KeyType, M - 1> keys;
            std::array<ValueType, M> children;
      };

      BTreeNode<KeyType, ValueType, M> m_root;
};
