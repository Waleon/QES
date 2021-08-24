/*
* X.509 CRL
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_X509_CRL_H_
#define BOTAN_X509_CRL_H_

#include <botan/x509_obj.h>
#include <botan/x509_dn.h>
#include <botan/crl_ent.h>
#include <vector>

namespace Botan {

class Extensions;
class X509_Certificate;

struct CRL_Data;

/**
* This class represents X.509 Certificate Revocation Lists (CRLs).
*/
class BOTAN_PUBLIC_API(2,0) X509_CRL final : public X509_Object
   {
   public:
      /**
      * This class represents CRL related errors.
      */
      class BOTAN_PUBLIC_API(2,0) X509_CRL_Error final : public Exception
         {
         public:
            explicit X509_CRL_Error(const std::string& error) :
               Exception("X509_CRL: " + error) {}
         };

      /**
      * Check if this particular certificate is listed in the CRL
      */
      bool is_revoked(const X509_Certificate& cert) const;

      /**
      * Get the entries of this CRL in the form of a vector.
      * @return vector containing the entries of this CRL.
      */
      const std::vector<CRL_Entry>& get_revoked() const;

      /**
      * Get the issuer DN of this CRL.
      * @return CRLs issuer DN
      */
      const X509_DN& issuer_dn() const;

      /**
      * @return extension data for this CRL
      */
      const Extensions& extensions() const;

      /**
      * Get the AuthorityKeyIdentifier of this CRL.
      * @return this CRLs AuthorityKeyIdentifier
      */
      const std::vector<uint8_t>& authority_key_id() const;

      /**
      * Get the serial number of this CRL.
      * @return CRLs serial number
      */
      uint32_t crl_number() const;

      /**
      * Get the CRL's thisUpdate value.
      * @return CRLs thisUpdate
      */
      const X509_Time& this_update() const;

      /**
      * Get the CRL's nextUpdate value.
      * @return CRLs nextdUpdate
      */
      const X509_Time& next_update() const;

      /**
      * Get the CRL's distribution point
      * @return CRL.IssuingDistributionPoint from the CRL's Data_Store
      */
      std::string crl_issuing_distribution_point() const;

      /**
      * Create an uninitialized CRL object. Any attempts to access
      * this object will throw an exception.
      */
      X509_CRL() = default;

      /**
      * Construct a CRL from a data source.
      * @param source the data source providing the DER or PEM encoded CRL.
      */
      X509_CRL(DataSource& source);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      /**
      * Construct a CRL from a file containing the DER or PEM encoded CRL.
      * @param filename the name of the CRL file
      */
      X509_CRL(const std::string& filename);
#endif

      /**
      * Construct a CRL from a binary vector
      * @param vec the binary (DER) representation of the CRL
      */
      X509_CRL(const std::vector<uint8_t>& vec);

      /**
      * Construct a CRL
      * @param issuer issuer of this CRL
      * @param thisUpdate valid from
      * @param nextUpdate valid until
      * @param revoked entries to be included in the CRL
      */
      X509_CRL(const X509_DN& issuer, const X509_Time& thisUpdate,
               const X509_Time& nextUpdate, const std::vector<CRL_Entry>& revoked);

   private:
      std::string PEM_label() const override;

      std::vector<std::string> alternate_PEM_labels() const override;

      void force_decode() override;

      const CRL_Data& data() const;

      std::shared_ptr<CRL_Data> m_data;
   };

}

#endif
