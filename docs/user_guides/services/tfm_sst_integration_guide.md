# TF-M Secure Storage Service Integration Guide

## Introduction

TF-M secure storage (SST) service allows storage of various types of data which
have security implications. It is meant to store the platform credentials
(keys, certificates, hashes, etc) that may require strict access controls.
The service is backed by hardware isolation of the flash access domain and, in
the current version, relies on hardware to isolate the flash area from
non-secure access. In absence of hardware level isolation, the secrecy and
integrity of data is still maintained.

The current SST service design relies on hardware abstraction level provided
by TF-M. The SST service provides a non-hierarchical storage model where all
the assets are managed by linearly indexed list of metadata. A minimal set of
APIs with limited options are provided. This allows a deterministic
implementation, better flash utilization and a code base which is easy to
security audit.

The SST service implements an AES-GCM based AEAD encryption policy, as a
reference, to protect data integrity and authenticity.

The design addresses the following high level requirements:

**Confidentiality** - Resistance to unauthorised accesses through
hardware/software attacks.

**Access Authentication** - Mechanism to establish requester's identity (a
non-secure entity, secure entity, or a remote server).

**Access Granularity** - Access permissions to create, write, read, delete and
reference an asset. Certain assets may be required to not be directly accessed
by an authorized client. It that case, the authorized client should be able to
reference the asset via another secure service.

**Integrity** - Resistant to tampering by either the normal users of a product,
package, or system or others with physical access to it. If the content of the
secure storage is changed maliciously, the service is able to detect it.

**Reliability** - Resistant to power failure scenarios and
incomplete write cycles.

**Configurability** - High level configurability to scale up/down memory
footprint to cater for a variety of devices with varying security requirements.

**Performance** - Optimized to be used for resource constrained devices with
very small silicon footprint, the PPA (power, performance, area) should be
optimal.

## Current SST Service Limitations

**Fragmentation** - The current design does not support fragmentation, as an
asset is stored in a contiguous space in a block. This means that the maximum
size of an asset can only be up-to a block size. Each block can potentially
store multiple assets. A delete operation implicitly moves all the assets
towards the top of the block to avoid fragmentation within block. However, this
may also result in unutilized space at the end of each block.

**Assets Size Limitation** - An asset is stored in a contiguous space in a
block/sector. Hence, the maximum size of an asset can be up-to the size of the
data block/sector.

**Protection against Physical Storage Mediums Failure** - Complete handling of
inherent failures of storage mediums (e.g. bad blocks in a NAND based device)
is not supported by the current design.

**Key Diversification** - In a more robust design, each asset would be encrypted
through a different key.

**Lifecycle Management** - Currently, it does not support any subscription based
keys and certificates required in a secure lifecycle management. Hence, an
asset's validity time-stamp can not be invalidated based on the system time.

**Provisioning vs User/Device Data** - In the current design, all assets are
treated in the same manner. In an alternative design, it may be required to
create separate partitions for provisioning content and user/device generated
content. This is to allow safe update of provisioning data during firmware
updates without the need to wipe out the user/device generated data.

## Code Structure

Secure storage service code is located in `secure_fw/services/secure_storage/`
and is divided as follows:

 - Core files
 - Flash filesystem interfaces
 - Flash interfaces
 - Cryptographic interfaces
 - Non-volatile (NV) counters interfaces
 - Assets definitions

The PSA interfaces for SST service are located in `interface/include`


### Platform Security Architecture (PSA) interfaces version 0.2

The SST service exposes the following PSA interfaces:

 - `enum psa_sst_err_t psa_sst_create(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size)`
 - `enum psa_sst_err_t psa_sst_get_info(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size, struct psa_sst_asset_info_t *info)`
 - `enum psa_sst_err_t psa_sst_get_attributes(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size, struct psa_sst_asset_attrs_t *attrs)`
 - `enum psa_sst_err_t psa_sst_set_attributes(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size, const struct psa_sst_asset_attrs_t *attrs)`
 - `enum psa_sst_err_t psa_sst_read(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size, uint32_t size, uint32_t offset, uint8_t *data)`
 - `enum psa_sst_err_t psa_sst_reference_read(int32_t  client_id, uint32_t asset_uuid, const uint8_t *token, uint32_t token_size, uint32_t size, uint32_t offset, uint8_t *data);`
 - `enum psa_sst_err_t psa_sst_write(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size, uint32_t size, uint32_t offset, const uint8_t *data)`
 - `enum psa_sst_err_t psa_sst_delete(uint32_t asset_uuid, const uint8_t *token, uint32_t token_size)`

These PSA interfaces and types are defined and documented
in `interface/include/psa_sst_api.h`, `interface/include/psa_sst_asset_defs.h`
and `interface/include/psa_sst_asset_macros.h`

### Core Files

`sst_object_system.c` - Contains the object system implementation to manage
all objects in SST area.

`sst_object_table.c` - Contains the object system table implementation which
complements the object system to manage all object in the SST area.
The object table has an entry for each object stored in the object system
and keeps track of its version.

`sst_encrypted_object.c` - Contains an implementation to manipulate
encrypted objects in the SST object system.

`sst_asset_management.c` - Contains asset's access policy management code.

`sst_utils.c` - Contains common and basic functionalities used across the
SST service code.

### Flash Filesystem Interface
`flash_fs/sst_flash_fs.h` - Abstracts the flash filesystem operations for
the secure storage service. The purpose of this abstraction is to have the
ability to plug-in other filesystems or filesystem proxys (supplicant).

`flash_fs/sst_flash_fs.c` - Contains the `sst_flash_fs` implementation for
the required interfaces.

`flash_fs/sst_flash_fs_mbloc.c` - Contains the metadata block manipulation
functions required to implement the `sst_flash_fs` interfaces in
`flash_fs/sst_flash_fs.c`.

`flash_fs/sst_flash_fs_mbloc.c` - Contains the data block manipulation
functions required to implement the `sst_flash_fs` interfaces in
`flash_fs/sst_flash_fs.c`.

The system integrator **may** replace this implementation with its own
flash filesystem implementation or filesystem proxy (supplicant).

### Flash Interface

`flash/sst_flash.h` - Abstracts the flash operations for the secure storage
service. It also defines the block size and number of blocks used by the SST
service.

`flash/sst_flash.c` - Contains the `sst_flash` implementation which sits on
top of CMSIS flash interface implemented by the target.
The CMSIS flash interface **must** be implemented for each target based on
its flash controller.

The block size (`SST_SECTOR_SIZE`) and number of blocks
(`SST_NBR_OF_SECTORS`) used by the secure storage area, are defined in
`flash_layout.h` located in `platform/ext/target/<TARGET_NAME>/partition`.
Those values **must** be defined in that header file based on flash
specifications and vendor specific considerations.
It is also required to define the `SST_FLASH_AREA_ADDR` which defines the
address of the first sector to be used as secure storage. The sectors reserved
to be used as secure storage **must** be contiguous sectors starting at
`SST_FLASH_AREA_ADDR`.

### Cryptographic Interface

`crypto/sst_crypto_interface.h` - Abstracts the cryptographic operations for
the secure storage service.

`crypto/sst_crypto_interface.c` - Currently, it implements the SST service
cryptographic operations using Mbed TLS library. The system integrator **may**
replace this implementation with calls to another service, crypto library or
hardware crypto unit.

### Non-volatile (NV) Counters Interface

`nv_counters/sst_nv_counters.h` - Abstracts SST non-volatile counters
operations. This API detaches the use of NV counters from the TF-M NV counters
implementation, provided by the platform, and provides a mechanism to compile
in a different API implementation for test purposes. A SST test suite **may**
provide its own custom implementation to be able to test different SST service
use cases.

`nv_counters/sst_nv_counters.c` - Implements the SST NV counters interfaces
based on TF-M NV counters implementation provided by the platform.

### Asset Definition

`asset/sst_asset_defs.(c/h)` - Contain a reference implementation of the
policy database of all assets in the system.

## SST Service Integration Guide

This section describes which interfaces **must** and **may** be implemented by
the system integrator in order to integrate the secure storage service in a new
platform.

### Secret Platform Unique Key

The encryption policy relies on a secret hardware unique key (HUK) per device.
It is system integrator's responsibility to provide an implementation which
**must** be a non-mutable target implementation.
For API specification, please check:
`platform/include/plat_crypto_keys.h`

A stub implementation is provided in
`platform/ext/sse_200_mps2/dummy_crypto_keys.c`

### Flash Interface

For SST service operations, a contiguous set of blocks must be earmarked for
the secure storage area. The design requires either 2 blocks, or any number of
blocks greater than or equal to 4. Total number of blocks can not be 0,1 or 3.
This is a design choice limitation to provide power failure safe update
operations.
For API specification, please check:
`secure_fw/services/secure_storage/flash/sst_flash.h`

### Asset Access Policy Management

Access to storage is governed by policy manager and policy database. The
policy manager is implemented in `sst_asset_management.c` and contains the
data driven access policy management code.
The policy database, currently, is linked into the code at the compile time.
The following files contain a reference implementation of policy database for
all assets in the system.
`secure_fw/services/secure_storage/assets/sst_asset_defs.h`
`secure_fw/services/secure_storage/assets/sst_asset_defs.c`

The system integrators **must** specify the policy database according to their
requirements by implementing those files with the proper content.

#### Policy Database Definition

`sst_asset_defs.h` defines the list of asset IDs, maximum size of each asset,
the client IDs allowed to access one or more assets, the number of assets
defined and the size of the largest asset.

The naming convention for those definition is as follows:

 - `SST_ASSET_ID_<ASSET NAME>` - To define an asset ID.
 - `SST_ASSET_MAX_SIZE_<ASSET NAME>` - To define the maximum size of the asset.
 - `SST_MAX_ASSET_SIZE` - To define the size of the largest asset defined in
   the file.
 - `SST_NUM_ASSETS` - To define the number of assets defined in the file.
 - `SST_CLIENT_ID_<N>` - To define a client ID number N which is allowed to
   access one or more assets.

The asset IDs and client IDs are unique in the definitions.

An example of `sst_asset_defs.h` definition is:
```
...
/* SST service reserved IDs */
#define SST_ASSET_ID_NO_ASSET 0
/* End SST service reserved IDs */

/* Asset IDs */
#define SST_ASSET_ID_AES_KEY_128 1
#define SST_ASSET_ID_AES_KEY_192 2

/* Asset sizes */
#define SST_ASSET_MAX_SIZE_AES_KEY_128 16
#define SST_ASSET_MAX_SIZE_AES_KEY_192 24

/* Client IDs which have access rights in one or more assets */
#define SST_CLIENT_ID_1 1
#define SST_CLIENT_ID_2 2

/* Number of assets that can be stored in SST area */
#define SST_NUM_ASSETS 2

/* Largest defined asset size */
#define SST_MAX_ASSET_SIZE 24
...
```

The policy table is composed by the asset information vector (`asset_perms`)
and the asset permissions mode vector (`asset_perms_modes`), in
`sst_asset_defs.c`. The asset information vector defines the properties of
all assets, while the asset permissions mode vector defines the access
permissions to those assets for each client. By default, if a client ID is
not defined for an specific asset in the `asset_perms_modes`, the asset is
not accessible for that client in any direct or referenced way.

The asset information structure (`struct sst_asset_policy_t`) contains the
following items:

 - `type` - PSA asset type
 - `asset_uuid` - Asset unique ID.
 - `max_size` - Asset maximum size.
 - `perms_count` - Number of clients specified in `asset_perms_modes`
   vector which have access rights for this asset in a direct or referenced
   way.
 - `perms_modes_start_idx` - First index in the `asset_perms_modes` vectors
   where the access permissions are defined for this specific asset.

The `struct sst_asset_info_t` definition can be found in
`secure_fw/services/secure_storage/sst_asset_management.h`

An example of `asset_perms` definition can be found below:

```
const struct sst_asset_policy_t asset_perms[] = {
{
    .type = PSA_SST_ASSET_KEY_AES,
    .asset_uuid = SST_ASSET_ID_AES_KEY_128,
    .max_size = SST_ASSET_MAX_SIZE_AES_KEY_128,
    .perms_count = SST_ASSET_PERMS_COUNT_AES_KEY_128,
    .perms_modes_start_idx = 0,
}, {
    .type = PSA_SST_ASSET_KEY_AES,
    .asset_uuid = SST_ASSET_ID_AES_KEY_192,
    .max_size = SST_ASSET_MAX_SIZE_AES_KEY_192,
    .perms_count = SST_ASSET_PERMS_COUNT_AES_KEY_192,
    .perms_modes_start_idx = 1,
}};
```

The asset permission structure (`struct sst_asset_perm_t`) is defined as
follows:

 - `client_id` - Client ID. ID for secure partitions is always bigger
   than 0. Non-secure clients use ID lower than 0. ID cannot be 0.
 - `perm` - Access permissions types, as a bit-field, allowed for this
   client.

The available access permission types are:

 - `SST_PERM_REFERENCE` - The client can request to a service to
   manipulate the asset content in its behalf.
 - `SST_PERM_READ` - The client can read the asset content.
 - `SST_PERM_WRITE` - The client can create, write and delete the asset.

The client permissions are defined in order for each asset. It means,
the first entries in the vector define the access permissions for all
the clients which have access to the first asset in `asset_perms`.
Then, the access permissions for the clients which have access to the
second asset, etc.
The policy manager retrieves the client permissions for each asset by
reading the proper entry in `asset_perms_modes` vector, based on
`.perms_modes_start_idx` and `.perms_count` asset properties in
`asset_perms`.

The `struct sst_asset_perm_t` definition can be found in
`secure_fw/services/secure_storage/sst_asset_management.h`

An example of `asset_perms_modes` definition can be found below:

```
const struct sst_asset_perm_t asset_perms_modes[] = {
{
    .client_id = SST_CLIENT_ID_1,
    .perm = SST_PERM_REFERENCE | SST_PERM_READ,
}, {
    .client_id = SST_CLIENT_ID_2,
    .perm = SST_PERM_REFERENCE | SST_PERM_READ | SST_PERM_WRITE,
} };
```

#### Generate a new policy database

The Python script `gen_policy_db.py` can be used to generate a new policy
DB based on the policy DB description in YAML format. The script is
available in `tools/services/sst/policy_db`. This Python script generates
automatically the `sst_asset_defs.h` and `sst_asset_defs.c` files which will be
used in the SST service build process to compile in the policy DB.

To define a policy for an asset, the asset definition must contain the
following fields in the YAML file:

 - `type` - PSA asset type define name defined as a string.
 - `name` - Asset name defined as a string.
 - `uuid` - Asset unique ID defined as an integer.
 - `max_size` - Asset maximum size defined as an integer.
 - `client_perms` - List of client IDs which have access to this asset.
 - `client_id` - Client ID defined as an integer.
 - `perms` - List of access permissions types allowed for this client. Each
   permission defined as a string.

The available access permission strings are:

 - `REFERENCE` - The client can request to a service to
   manipulate the asset content in its behalf.
 - `READ`  - The client can read the asset content.
 - `WRITE` - The client can create, write and delete the asset.

An example of asset definition in YAML format can be found below:

```
...
{
  "type": "PSA_SST_ASSET_KEY_HMAC",
  "name": "AES_KEY_128",
  "uuid": 1,
  "max_size": 16,
  "client_perms": [
    {
      "client_id": 1,
      "perms":  ["REFERENCE","READ"]
    }, {
      "client_id": 2,
      "perms":  ["REFERENCE","READ","WRITE"]
    }
  ]
},
...
```

The following command can be executed to generate the new policy DB files based
on a new YAML file description with `gen_policy_db.py`.
```
python gen_policy_db.py -i new_policy_db.yaml
```

Detailed information about how to use `gen_policy_db.py` can be found by
executing the script with -h parameter.
```
python gen_policy_db.py -h
```

The current policy DB is defined in `tfm_sst_policy_db_definition.yaml`
located in `tools/services/sst/policy_db` folder.

### Non-Secure Identity Manager

TF-M core tracks the current client IDs running in the secure or non-secure
processing environment. It provides a dedicated API to retrieve the client ID
which performs the service request.

[ns client identification documentation](../tfm_ns_client_identification.md)
provides further details on how client identification works.

SST service uses that TF-M core API to retrieve the client ID and validate the
access permission against the requested asset.

The [integration guide](../tfm_integration_guide.md) provides further
details of non-secure implementation requirements for TF-M.

### Cryptographic Interface

The reference encryption policy is built on AES-GCM, and it **may** be replaced
by a vendor specific implementation.
The SST service abstracts all the cryptographic requirements and specifies the
required cryptographic interface in
`secure_fw/services/secure_storage/crypto/sst_crypto_interface.h`

Currently, the SST service cryptographic operations are implemented in
`secure_fw/services/secure_storage/crypto/sst_crypto_interface.c`, using
Mbed TLS library.

### SST Service Features Flags

SST service defines a set of flags that can be used to compile in/out certain
SST service features. The **CommonConfig.cmake** file sets the default values
of those flags. However, those flags values can be overwritten by setting them
in `platform/ext/<TARGET_NAME>.cmake` based on the target capabilities or needs.
The list of SST services flags are:

 - `ENABLE_SECURE_STORAGE`: this flag allows to compile in/out the secure
   storage service.
 - `SST_ENCRYPTION`: this flag allows to enable/disable encryption option to
   encrypt the secure storage data.
 - `SST_CREATE_FLASH_LAYOUT`: this flag indicates that it is required to
   create a SST flash layout. If this flag is set, SST service will generate an
   empty and valid SST flash layout to store assets. It will erase all data
   located in the assigned SST memory area before generating the SST layout.
   This flag is required to be set if the SST memory area is located in a
   non-persistent memory.
   This flag can be set if the SST memory area is located in a persistent
   memory without a valid SST flash layout in it. That is the case when
   it is the first time in the device life that the SST service is executed.
 - `SST_VALIDATE_METADATA_FROM_FLASH`: this flag allows to enable/disable the
   validation mechanism to check the metadata store in flash every time the
   flash data is read from flash. This validation is required if the flash is
   not hardware protected against malicious writes. In case the flash is
   protected against malicious writes (i.e embedded flash, etc), this validation
   can be disabled in order to reduce the validation overhead.
 - `SST_ENABLE_PARTIAL_ASSET_RW`: this flag allows to enable/disable the
   partial asset RW manipulation at compile time. The partial asset
   manipulation is allowed by default.
 - `SST_ROLLBACK_PROTECTION`: this flag allows to enable/disable rollback
   protection in secure storage service. This flag takes effect only if the
   target has non-volatile counters and `SST_ENCRYPTION` flag is on.
 - `SST_RAM_FS`: this flag allows to enable/disable the use of RAM instead
   of the flash to store the FS in secure storage service. This flag is set
   by default in the regression tests, if it is not defined by the platform.
   The SST regression tests reduce the life of the flash memory as they
   write/erase multiple times in the memory.

--------------

*Copyright (c) 2018, Arm Limited. All rights reserved.*
